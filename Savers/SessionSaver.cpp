// Copyright 2018

#include "SessionSaver.h"
#include "Logger.h"
#include "Utils.h"
#include "Defines.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>

namespace VMM
{
	using namespace boost::filesystem;
	using namespace boost::system;

	SessionSaver::SessionSaver(const std::string& aFolderPath) :
		mFolderPath(aFolderPath)
    {
        Utils::CheckPath(mFolderPath);
	}

	SessionSaver::~SessionSaver()
	{
        Logger::Instance().RemoveCallback(mLoggerUserName);

        StopThread();
	}

    std::string 
	SessionSaver::GetFileName(FileType aFileType)
    {
        switch (aFileType)
        {
        case FileType::Log:
            return "log_" + mStartDateTime + ".txt";
        default:
            return "";
        }
    }

    void 
	SessionSaver::InitFile(FileType aFileType, bool aAppend)
    {
        std::string fileName = GetFileName(aFileType);
        if (fileName.empty())
        {
            LOGGER_LOG(PriorityEnum::Error, Utils::Format("Init file %d failed!", aFileType));
        }
        else
        {
            std::unique_ptr<FileSaver> file(new FileSaver());
            std::string filePath = mFolderPath + mSessionFolderName + fileName;
            if (file->InitFileSaver(filePath, aAppend))
            {
                std::lock_guard<std::mutex> lock(mFilesMapMutex);
                mFilesMap.emplace(aFileType, std::move(file));
            }
            else
            {
                LOGGER_LOG(PriorityEnum::Error, "Init file " + filePath + " failed!");
            }
        }
    }

    void SessionSaver::ClearFileMap()
    {
        std::lock_guard<std::mutex> lock(mFilesMapMutex);
        // Clear file map
        mFilesMap.clear();
    }

	bool 
	SessionSaver::InitSessionSaver(const std::string& aSessionFolderName, const std::string& aStartDateTime, WrittenFrameCallback aWriteFrameCallback, uint64_t aMaxDataSize)
	{
        mMaxDataSize = aMaxDataSize;
        mStartDateTime = aStartDateTime;
        // mSessionFolderName = aSessionFolderName + aStartDateTime;
//#ifdef _WIN32
        mSessionFolderName = Utils::CheckPathCopy(aSessionFolderName + aStartDateTime);
//#endif
		mWriteFrameCallback = aWriteFrameCallback;

		try
		{   
            // Stop processing thread if it was started
            StopThread();

            // Init folder manager
			if (!mFolderManager.Init(mFolderPath, mSessionFolderName, aSessionFolderName))
			{
				return false;
			}

            ClearFileMap();

            // Init log file
            InitFile(FileType::Log, true);

            // Generate unique logger user id
            mLoggerUserName = "SessionSaver" + aStartDateTime;

            // Subscribe to log mesaages
            Logger::Instance().AddCallback(mLoggerUserName, std::bind(&SessionSaver::OnLoggerMessage, this, std::placeholders::_1));

            // Save current data size
			// auto size = mFolderManager.GetCurrentDataSize();
			// mCurrentDataSize += size;

            // Start processing thread
			if (!StartThread())
			{
				LOGGER_LOG(PriorityEnum::Error, "Start Session save thread for " + mFolderPath + mSessionFolderName + " failed!");
				return false;
			}
		}
		catch (const std::exception& ex)
		{
			LOGGER_LOG(PriorityEnum::Error, ex.what());
			return false;
		}
		return true;
	}

	bool 
	SessionSaver::HandleData(std::shared_ptr<Data> aData, size_t aMaxQueueSize)
	{
        if (!aData || aData->data.empty())
        {
            // Skip empty packets
            return false;
        }

		return AddItemSkip(aData, aMaxQueueSize);
	}

	void 
	SessionSaver::HandleWorkItem(std::shared_ptr<Data> aItem)
	{
		size_t dataSize = aItem->data.size();
		size_t freeSize = 0;

		/* ЛОГИКА ЦИКЛИЧЕСКОЙ ПОСЕАНСОВОЙ ПЕРЕЗАПИСИ
		if (mFolderManager.IsOtherFolderExists() && mCurrentDataSize + dataSize > mMaxDataSize)
		{
		freeSize += mFolderManager.RemoveOtherFolder();
		}

		while (mFolderManager.IsAnyOldFolderExists() && mCurrentDataSize - freeSize + dataSize > mMaxDataSize)
		{
		freeSize += mFolderManager.RemoveOldestFolder();
		}

		if (mCurrentDataSize - freeSize + dataSize > mMaxDataSize)
		{
		freeSize += mSessionSaver.FreeData(mCurrentDataSize + dataSize - mMaxDataSize);
		}

		mCurrentDataSize -= freeSize;*/


		// если писать некуда, то не пишем и выкидываем сообщение об ошибке
		if (mCurrentDataSize + dataSize <= mMaxDataSize)
		{
			auto writtenSize = HandleFrameData(aItem);
			if (writtenSize > 0)
			{
				mCurrentDataSize += dataSize;
			}
			else
				freeSize = 1;
		}
		else
			freeSize = 2;

		// теперь freeSize используется в контексте что записи не произошло так как писать больше некуда либо ошибка при записи			
		if (mWriteFrameCallback)
			mWriteFrameCallback(mFolderPath + mSessionFolderName, freeSize);
	}

    void 
    SessionSaver::OnLoggerMessage(std::shared_ptr<Logger::LogEntry> aLogMessage)
    {
        WriteFile(FileType::Log, aLogMessage->Print());
    }

    bool 
	SessionSaver::WriteFile(FileType aFileType, const std::string& aData)
    {
        std::lock_guard<std::mutex> lock(mFilesMapMutex);

        auto itr = mFilesMap.find(aFileType);
        if (itr != mFilesMap.end())
        {           
            return itr->second->WriteData(aData);
        }
        return false;
    }

	uint64_t 
	SessionSaver::GetCurrentFreeSize()
	{
		return (mMaxDataSize - mCurrentDataSize);
	}

	size_t SessionSaver::HandleFrameData(std::shared_ptr<Data> aData)
	{
		size_t dataSize = aData->data.size();

		if (mMaxFrameCounter != 0u && mFrameCounter == mMaxFrameCounter)
		{
			// Reset frame counter 
			mFrameCounter = 0u;
		}

		// Remove old file with same frame counter if exists
		auto itr = mFramesMap.find(++mFrameCounter);
		if (itr != mFramesMap.end())
		{
			remove(itr->second.c_str());
		}

		// Save frame
		std::string filePath = mFolderPath + mSessionFolderName + Utils::Format("%.6u_", mFrameCounter) + Utils::FormatFileName(aData->time) + FileExtension;
		std::ofstream file(filePath, std::ios::out | std::ios::binary);
		if (file.is_open())
		{
			file.write((const char*)&aData->data[0], dataSize);

			file.close();
			if (!file)
			{
				// error handling
				LOGGER_LOG(PriorityEnum::Error, "Save frame for " + mFolderPath + " failed!");
				dataSize = 0;
			}
			else
			{
				// Add writtn file to file map
				mFramesMap[mFrameCounter] = filePath;
			}
		}
		else
		{
			LOGGER_LOG(PriorityEnum::Error, "Save frame for " + mFolderPath + " failed!");
			dataSize = 0;
		}

		return mMaxFrameCounter == 0 ? dataSize : 0;
	}

	uint64_t SessionSaver::FreeFrameData(uint64_t aRequiredSize)
	{
		uint64_t dataRemoved = 0;

		if (mFolderManager.IsOtherFolderExists())
		{
			// reduce required size
			dataRemoved += mFolderManager.RemoveOtherFolder();
		}

		if (mMaxFrameCounter == 0)
		{
			// Remove old sessions until get required free space
			while (aRequiredSize - dataRemoved > 0 && mFolderManager.IsAnyOldFolderExists())
			{
				// reduce required size
				dataRemoved += mFolderManager.RemoveOldestFolder();
			}

			if (aRequiredSize - dataRemoved > 0)
			{
				mMaxFrameCounter = mFrameCounter;
			}
		}

		return dataRemoved;
	}
}
