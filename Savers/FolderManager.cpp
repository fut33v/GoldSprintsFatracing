// Copyright 2018

#include "FolderManager.h"
#include "Utils.h"
#include "Logger.h"

#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace VMM
{
    using namespace boost::filesystem;
    using namespace boost::system;

    FolderManager::FolderManager()
    {
    }
    
    FolderManager::~FolderManager()
    {
    }

    bool
	FolderManager::Init(const std::string& aFolderPath, const std::string& aCurrentFolderName, const std::string& aLegalFolderNameTemplate)
    {
        mFolders.clear();
        mFolderPath = Utils::CheckPathCopy(aFolderPath);
        // Check and create main folder if needed

        path mainFolderPath(mFolderPath);
        error_code error;
        if (exists(mainFolderPath, error) && is_directory(mainFolderPath, error))
        {
            try
            {
				// sort included subfolders by template

                for (auto& x : directory_iterator(mainFolderPath))
                {
                    if (is_directory(x, error))
                    {
                        if (!aLegalFolderNameTemplate.empty() && x.path().string().find(aLegalFolderNameTemplate) == std::string::npos)
                        {
                            mFoldersOther.emplace_back(x);
                        }
                        else
                        {
                            mFolders.emplace_back(x);
                        }
                    }
                }

                if (!mFolders.empty()) mFolders.sort();
                if (!mFoldersOther.empty()) mFoldersOther.sort();
            }
            catch (const std::exception& ex)
            {
                LOGGER_LOG(PriorityEnum::Error, ex.what());
                return false;
            }
        }
        else
        {
            if (!create_directory(mainFolderPath, error))
            {
                LOGGER_LOG(PriorityEnum::Error, "Create directory " + mainFolderPath.string() + " failed!");
                return false;
            }
        }

        // Check and create current folder
        auto folder = Utils::GetFolderPath(mFolderPath + aCurrentFolderName);

        // Create folder 
        if (!create_directory(folder, error))
        {
            LOGGER_LOG(PriorityEnum::Error, "Create directory " + folder.string() + " failed!");
            return false;
        }
        
        // Add current foler to folders list
        mFolders.emplace_back(folder);

        // Save current folder path
        mCurrentFolderPath = folder;
        return true;
    }

    uint64_t
	FolderManager::RemoveFolder(boost::filesystem::path aFolderPath)
    {       
        // get folder size on disk
        uint64_t folderSize = 0;
        try
        {
            folderSize = Utils::GetFolderSize(aFolderPath);
        }
        catch (const std::exception& ex)
        {
            LOGGER_LOG(PriorityEnum::Warning, ex.what());
			folderSize = 0;
        }

        if (folderSize != 0)
        {
            // remove folder on specified path
            error_code error;
			uintmax_t remSize = 0;
			try
			{
				remSize = remove_all(aFolderPath, error);
				folderSize = static_cast<uint64_t>(remSize);
			}
			catch (const std::exception& ex)
			{
				LOGGER_LOG(PriorityEnum::Warning, ex.what());
				folderSize = 0;
			}

            if (error)
            {
                folderSize = 0;
            }
        }
        
        return folderSize;
    }

    uint64_t
	FolderManager::RemoveOldestFolder()
    {
        uint64_t removedDataSize = 0u;

        if (!mFolders.empty())
        {
            // Remove old Session folder
            path foldertPath = mFolders.front();
            if (foldertPath != mCurrentFolderPath)
            {
                removedDataSize += RemoveFolder(foldertPath);                
                mFolders.pop_front();
            }            
        }
        return removedDataSize;
    }

	uint64_t 
	FolderManager::RemoveOtherFolder()
	{
		uint64_t removedDataSize = 0u;

		if (!mFoldersOther.empty())
		{
			// Remove illegal stuff
			for (auto& x : mFoldersOther)
			{
				removedDataSize += RemoveFolder(x);
			}

			// Clear all removed folders, don't try to remove folders that can't be removed
			mFoldersOther.clear();
		}
		
		return removedDataSize;
	}

    uint64_t
	FolderManager::GetCurrentDataSize()
    {
        uint64_t dataSize = 0u;

        // Get all other folders size
        for (auto& x : mFoldersOther)
        {
            try
            {
                dataSize += Utils::GetFolderSize(x);
            }
            catch (const std::exception& ex)
            {
                LOGGER_LOG(PriorityEnum::Warning, ex.what());
            }
        }

        // Get all Session folders size
        for (auto& x : mFolders)
        {
            if (x == mCurrentFolderPath)
            {
                break;
            }

            try
            {
                dataSize += Utils::GetFolderSize(x);
            }
            catch (const std::exception& ex)
            {
                LOGGER_LOG(PriorityEnum::Warning, ex.what());
            }
        }
        return dataSize;
    }

    bool
	FolderManager::IsAnyOldFolderExists()
    {
        return !mFolders.empty() && mFolders.front() != mCurrentFolderPath;
    }       

	bool
	FolderManager::IsOtherFolderExists()
	{
		return !mFoldersOther.empty();
	}
}
