// Copyright 2018

#include "DiskManager.h"

#ifdef _WIN32
#include "Win32/DiskUtils.h"
#elif __linux__
#include "Linux/DiskUtils.h"
#endif

#include "Utils.h"
#include "Defines.h"
#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>

namespace VMM
{
    using namespace boost::filesystem;
    using namespace boost::system;

    DiskManager::DiskManager(uint64_t aMaxDataSize) : mMaxDataSize(aMaxDataSize) {
    }
    
    DiskManager::~DiskManager() {
    }

    size_t 
    DiskManager::InitDisks(const std::vector<std::string> aDiskLabels, FrameWroteCallback aFrameWroteCallback) {
        mDrives.clear();
        mDriveFilesCount.clear();
        mFrameWroteCallback = aFrameWroteCallback;
        // Enum fixed disks
        std::vector<std::string> drives = DiskUtils::GetFixedDrives();
        if (drives.empty()) {
            LOGGER_LOG(PriorityEnum::Error, "Не найдено дисков подходящего размера!");
            return 0;
        }
        size_t diskNumber = 0;

        // Create Session folders for each suitable disk
        for (auto& drive : drives) {
            std::string label = DiskUtils::GetDriveLabel(drive);
			
            ptrdiff_t pos = std::find(aDiskLabels.begin(), aDiskLabels.end(), label) - aDiskLabels.begin();
            if ((unsigned long long)pos < aDiskLabels.size()) // диск с меткой найден
            {


                bool mounted = true;
#ifdef _WIN32
                path drivePath(drive);
#elif __linux__
                std::string mountPoint;
                if (!DiskUtils::GetMountPoint(drive, mountPoint)) {
                    mounted = false;
                }
                path drivePath(mountPoint);
#endif

                error_code error;
                space_info info = space(drivePath, error);
                if (!error && mounted)
                {
                    if (info.capacity >= mMaxDataSize)
                    {
                        uint64_t unusableSize = info.capacity - mMaxDataSize;
                        mUnusableSizeDiskMap.emplace(drive, unusableSize);
						
                        mDrives.push_back(drive);

                        /*
                        auto cntFiles = Utils::GetFileCount(drivePath, FileExtension);
                        mFileCount += cntFiles;
                        mDriveFilesCount.push_back(cntFiles);
                        */

                        diskNumber += (pos + 1);
                    }
                }
                else
                {
                    LOGGER_LOG(PriorityEnum::Error, "Не удалось получить свободное место! Диск " + drive);
                }

               }
        }

        if (mDrives.empty())
        {
            LOGGER_LOG(PriorityEnum::Error, "Не удалось найти диски, загрузка дисков неуспешна!");
            diskNumber = 0;
        }

        return diskNumber;
    }

    bool DiskManager::StartSession() {
        if (mDrives.empty()) {
            LOGGER_LOG(PriorityEnum::Error, "Загрузка фиксированных дисков неуспешна, список пуст!");
            return false;
        }
		
        // Generate Session folder name
        tm t = Utils::TimeToTimeT(std::chrono::system_clock::now());
        std::string SessionFolderName = FolderName + "_";
        std::string SessionStartDateTime = Utils::Format("%04u-%02u-%02u_%02u.%02u.%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

		std::lock_guard<std::mutex> lock(mSessionSaversMutex);

		mSessionSavers.clear();

        // Create Session folders for each siutable disk
        for (auto& drive : mDrives) {
            std::string path;
#ifdef _WIN32
            path = drive;
#elif __linux__
            std::string mountPoint;
            if (!DiskUtils::GetMountPoint(drive, mountPoint)) {
                continue;
            }
            path = mountPoint;
#endif

            std::unique_ptr<SessionSaver> saver(new SessionSaver(path));
            if (saver->InitSessionSaver(SessionFolderName, SessionStartDateTime, 
				std::bind(&DiskManager::WrittenFrame, this, std::placeholders::_1, std::placeholders::_2),
				mMaxDataSize))
            {
                mSessionSavers.emplace(drive, std::move(saver));				
            }
        }

        if (!mSessionSavers.empty())
        {
            mSessionSaversItr = mSessionSavers.begin();
            return true;
        }
        return false;
    }

    bool 
	DiskManager::HandleData(std::shared_ptr<Data> aData, size_t aMaxQueueSize)
    {
        if (!aData || aData->data.empty())
        {
            // Skip invalid packets
            return false;
        }

        std::lock_guard<std::mutex> lock(mSessionSaversMutex);
        
        bool result = false;
        if (!mSessionSavers.empty())
        {
            // Handle data for current disk
            result = mSessionSaversItr->second->HandleData(aData, aMaxQueueSize);

            // Switch to next disk
            if (++mSessionSaversItr == mSessionSavers.end())
            {
                mSessionSaversItr = mSessionSavers.begin();
            }
        }
        return result;
    }

    void 
	DiskManager::WriteToFile(FileType aFileType, const std::string& aData)
    {
        std::lock_guard<std::mutex> lock(mSessionSaversMutex);

        for (auto& x : mSessionSavers)
        {
            x.second->WriteFile(aFileType, aData);
        }
    }

    std::map<std::string, uint64_t>
	DiskManager::FreeSpace()
    {
        std::map<std::string, uint64_t> result;

        for (auto& drive : mDrives)
        {
            error_code error;

#ifdef _WIN32
            space_info info = space(path(drive), error);
#elif __linux__
            std::string mountPoint;
            if (!DiskUtils::GetMountPoint(drive, mountPoint)) {
                continue;
            }
            space_info info = space(path(mountPoint), error);
#endif
            if (!error)
            {
				uint64_t unusablesize = mUnusableSizeDiskMap.at(drive);
				int64_t realFree = info.free - unusablesize;				
				if (realFree < 0)
					realFree = 0;
                result.emplace(drive, realFree);
            }
        }
        return result;
    }

	std::vector<uint64_t>
	DiskManager::GetDiskFilesCount()
    {		
        std::lock_guard<std::mutex> lock(mSessionSaversMutex);
        return mDriveFilesCount;
	}

	uint64_t
	DiskManager::GetFileCount()
	{
		std::lock_guard<std::mutex> lock(mSessionSaversMutex);
		return mFileCount;
	}	

	void 
	DiskManager::WrittenFrame(std::string aDrive, uint64_t aErrorCode)
    {
		{
			std::lock_guard<std::mutex> lock(mSessionSaversMutex);
			if (aErrorCode == 0) // No Errors
				mFileCount++;
		}
		
		if (mFrameWroteCallback)
			mFrameWroteCallback(aErrorCode);
    }

	uint64_t
	DiskManager::GetFreeRecorderSize()
    {	  
		uint64_t result = 0;

		for (auto& drive : mDrives)
		{
            error_code error;


#ifdef _WIN32
            space_info info = space(path(drive), error);
#elif __linux__
            std::string mountPoint;
            if (!DiskUtils::GetMountPoint(drive, mountPoint)) {
                continue;
            }
            space_info info = space(path(mountPoint), error);
#endif

			if (!error)
			{
				uint64_t unusablesize = mUnusableSizeDiskMap.at(drive);
				result += info.free - unusablesize;
			}
		}
		return result;
    }
}
