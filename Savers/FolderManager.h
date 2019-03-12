// Copyright 2018

#pragma once

#include <boost/filesystem/path.hpp>

namespace VMM
{
    class FolderManager
    {
        //! Путь к текущей папке
        std::string mFolderPath;
        //! Путь к текущей подпапке (ислючена из удаления)
        boost::filesystem::path mCurrentFolderPath;
        //! Подпапки подходящие под заданный шаблон
        std::list<boost::filesystem::path> mFolders;
        //! Подпапки c нелегальным содержимым (не подходящие под шаблон)
        std::list<boost::filesystem::path> mFoldersOther;
            
    public:
        FolderManager();
        ~FolderManager();

        //! Инициализация
        bool Init(const std::string& aFolderPath, const std::string& aCurrentFolderName, const std::string& aLegalFolderNameTemplate = "");
        //! Проверка наличия старой папки (исключая текущую)
        bool IsAnyOldFolderExists();
		//! Проверка наличия посторонних папок
		bool IsOtherFolderExists();
        //! Удаление старых подпапок (исключая текущую)
        uint64_t RemoveOldestFolder();
		//! Удаление папок, несоответвующих ранее заданному шаблону
		uint64_t RemoveOtherFolder();
        //! Текущий размер данных
        uint64_t GetCurrentDataSize();

    private:
        uint64_t RemoveFolder(boost::filesystem::path aFolderPath);
    };
}

