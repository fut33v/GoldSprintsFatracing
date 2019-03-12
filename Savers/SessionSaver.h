// Copyright 2018

#pragma once

#include "SessionSaver.h"
#include "FileSaver.h"
#include "FolderManager.h"
#include "CommonStructures.h"
#include "AsyncQueue.h"
#include "Logger.h"

#include <map>
#include <string>

namespace Fatracing
{
	typedef std::function<void(std::string /*drive*/, uint64_t /* last free size*/)> WrittenFrameCallback;

    class SessionSaver : protected AsyncQueue<Data>
    {
        //! Менеджер данных в папке
        FolderManager mFolderManager; 

        //! Путь к папке полета
        std::string mFolderPath;
        //! Имя папки текущего полета
        std::string mSessionFolderName;
        //! Строка даты / времени текущего полета
        std::string mStartDateTime;
        //! Максимальный размер данных
        uint64_t mMaxDataSize = 0u;
        //! Текущий размер данных
        uint64_t mCurrentDataSize = 0u;

		// Счетчик кадров
		unsigned long mFrameCounter = 0u;
		// Максимальное значение счетчика кадров
		unsigned long mMaxFrameCounter = 0u;
		// Карта кадров
		std::map<unsigned long, std::string> mFramesMap;

        // Мьютекс карты файлов
        std::mutex mFilesMapMutex;
        //! Карта дополнительных инфо файлов
        std::map<FileType, std::unique_ptr<FileSaver>> mFilesMap;

        std::string mLoggerUserName;

		WrittenFrameCallback mWriteFrameCallback;

    private:
        //! Получить имя доп файла
        std::string GetFileName(FileType aFileType);
        //! Инициализация доп файла
        void InitFile(FileType aFileType, bool aAppend = false);
        //! Очистка карты файлов
        inline void ClearFileMap();

		// Обработка кадра
		uint64_t HandleFrameData(std::shared_ptr<Data> aData);
		// Освобождение места
		uint64_t FreeFrameData(uint64_t aRequiredSize);

    protected:
        void HandleWorkItem(std::shared_ptr<Data> aItem) override;
        // Обработка сообщения из логгера
        void OnLoggerMessage(std::shared_ptr<Logger::LogEntry> aLogMessage);

    public:
        SessionSaver(const std::string& aSaveFolderPath);
        ~SessionSaver();

        //! Инициализация полета
        bool InitSessionSaver(const std::string& aSessionFolderName, const std::string& aStartDateTime, WrittenFrameCallback aWriteFrameCallback, uint64_t aMaxDataSize);
        //! Обработка кадра
        bool HandleData(std::shared_ptr<Data> aData, size_t aMaxQueueSize = 0);
        //! Запись строки в доп файл
        bool WriteFile(FileType aFileType, const std::string& aData);
		//! Получить размер оставшегося свободного пространства
		uint64_t GetCurrentFreeSize();
    };
}

