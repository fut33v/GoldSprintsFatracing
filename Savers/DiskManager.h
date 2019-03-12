// Copyright 2018

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <vector>

#include "SessionSaver.h"
#include "CommonStructures.h"
#include "Logger.h"

namespace Fatracing
{
	typedef std::function<void(uint64_t /* last free size*/)> FrameWroteCallback;

    class DiskManager
    {        
        //! Количество файлов
        uint64_t mFileCount = 0u;
        //! Максимальный размер данных на диске
        uint64_t mMaxDataSize = 0u;
		//! Размер неиспользуемого пространства для каждого диска
		std::map<std::string, uint64_t> mUnusableSizeDiskMap;		
        // Счетчик сессий
        uint32_t mSessionCounter = 0u;
        // Мьютекс карты полетов
        std::mutex mSessionSaversMutex;
        // Карта полетов
        std::map<std::string, std::unique_ptr<SessionSaver>> mSessionSavers;
        // Итератор текущего полета
        std::map<std::string, std::unique_ptr<SessionSaver>>::iterator mSessionSaversItr;
        // Используемые диски
        std::vector<std::string> mDrives;
		// Используемые диски
		std::vector<uint64_t> mDriveFilesCount;

		FrameWroteCallback mFrameWroteCallback;

    public:
        // TODO: set right number
        DiskManager(uint64_t aMaxDataSize = 500000000 /*15ull << 30ull*/);
        ~DiskManager();
        
        //! Инициализация дисков, возвращает количество инициализированных дисков
        size_t InitDisks(const std::vector<std::string> aDiskLabels, FrameWroteCallback aFrameWroteCallback = nullptr);
        //! Старт полета
        bool StartSession();
        //! Сохранение кадра
        bool HandleData(std::shared_ptr<Data> aData, size_t aMaxQueueSize = 0);
        //! Сохранение данных в файл
        void WriteToFile(FileType aFileType, const std::string& aData);
        //! Получить свободное место по всем используемым дискам
        std::map<std::string, uint64_t> FreeSpace();
        //! Получить количество файлов
        uint64_t GetFileCount();
		//! получить количество найденных контейнеров изображений по дискам 
		std::vector<uint64_t> GetDiskFilesCount();

	private:
		void WrittenFrame(std::string aDrive, uint64_t aErrorCode);
		uint64_t GetFreeRecorderSize();
    };
}

