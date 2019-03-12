// Copyright 2018

#ifndef SUBPRJ_COMMON_BASE_LOGGER_H_
#define SUBPRJ_COMMON_BASE_LOGGER_H_

#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include "Convert.h"
#include "Utils.h"
#include <atomic>
#include <iostream>

namespace VMM
{
//! Приоритет записи лога
enum class PriorityEnum : int
{
	Trace = 0,
	Debug = 1,
	Info = 2,
	Warning = 3,
	Error = 4,
	Success = 5
};

//! Логгер
class Logger
{
public:
	//! Запись лога
	struct LogEntry
	{
		//! Конструктор
		LogEntry(std::chrono::system_clock::time_point aTime, PriorityEnum aPriority, std::string aMessage,
		         std::string aFile = "", std::string aFunction = "", int aLine = 0)
		{
			Time = aTime;
			// TimeString = Utils::Format(aTime);
			Priority = aPriority;
			Message = aMessage;
			File = aFile;
			Function = aFunction;
			Line = aLine;
		}

		LogEntry(): Priority(PriorityEnum::Debug), Line(0)
		{
		}

		LogEntry(const LogEntry& other)
		{
			Time = other.Time;
			Priority = other.Priority;
			Message = other.Message;
			File = other.File;
			Function = other.Function;
			Line = other.Line;
			Counter.store(other.Counter.load());
		}

		LogEntry& operator=(const LogEntry& other)
		{
			Time = other.Time;
			Priority = other.Priority;
			Message = other.Message;
			File = other.File;
			Function = other.Function;
			Line = other.Line;
			Counter.store(other.Counter.load());
			return *this;
		}

		std::string Print() const
		{
			return Utils::Format(Time) + PriorityToString(Priority) + " " + File + "(" + std::to_string(Line) + ")::" + Function
				+
				"(): " + Message + (Counter > 0 ? " [" + std::to_string(Counter) + "]" : "");
		}

		//! Время
		std::chrono::system_clock::time_point Time;
		//! Приоритет
		PriorityEnum Priority;
		//! Сообщение 
		std::string Message;
		//! Файл из которого был вызван логгер	
		std::string File;
		//! Функция из которой был вызван логгер
		std::string Function;
		//! Строка из которой был вызван логгер
		int Line;

		//! Счётчик повторений записи
		std::atomic<int> Counter{0};
	};

	//! Колбек для передачи лога в GUI
	typedef std::function<void(std::shared_ptr<LogEntry>)> LoggerCallback;

private:
	//! Конструктор
	Logger();
	//! Деструктор
	~Logger();

	//! Путь к файлу
	std::string LogFilePath;
	//! Расширение файла лога
	const char* LogFileExtension = ".log";
	//! Куда пишем лог
	std::ofstream mFileStream;

	//! Строки для приоритетов лога
	const std::vector<std::string> PriorityString = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "SUCCESS"};
	//! Мютекс файла
	std::mutex mMutex;
	//! Колбек для передачи логов в GUI
	std::map<std::string, LoggerCallback> mLoggerCallbacks;

	//! Пишет ли данный класс лог в файл
	std::atomic<bool> mWriteToFileEnabled{true};
	//! Пишет ли логгер в консоль
	std::atomic<bool> mWriteToConsoleEnabled{false};
	//! Последняя запись
	LogEntry mLastLogEntry;
	//! Уровень логирования
	PriorityEnum mLogLevel;

public:
	//! Получить экземпляр синглтона
	static Logger& Instance();

	//! Дамп
	void Dump();

	//! Добавить коллбек для событий логирования (для визуализации в GUI и передачи в другие модули)
	//! @param aCallback коллбек в который будут приходить лог сообщения
	void AddCallback(const std::string& aCallbackName, LoggerCallback aCallback);
    //! Удалить коллбек
    void RemoveCallback(const std::string& aCallbackName);
	//! Обнулить коллбеки событий логирования
	void ClearCallbacks();

	//! Установить запись в файл
	void SetWriteToFileEnabled(bool aWriteToFileEnabled);
	//! Установить запись в консоль
	void SetWriteToConsoleEnabled(bool aWriteToConsoleEnabled);
	//! Установить уровень логгирования
	void SetLogLevel(PriorityEnum aPriority);


	//! Преобразовать приоритет в строку
	static std::string PriorityToString(PriorityEnum aPriority);


	//! Логгирование, используйте LOGGER_LOG
	template <typename... Args>
	void Log(PriorityEnum aPriority, std::string aFile, std::string aFunction, int aLine, std::string format,
	         Args&&... vs);

#ifdef __linux__
    // template <typename... Args>
    // static void LogStatic(PriorityEnum aPriority, std::string aFile, std::string aFunction, int aLine, std::string format) {
    //      Args&&... vs
    //     Logger::Instance().Log(aPriority, aFile, aFunction, aLine, format, vs...);
    // }

#endif

private:
	//! Обработать запись лога, передать другим модулям
	void ProcessLog(std::shared_ptr<LogEntry> logEntry);

	//! Открыть файл на запись
	bool OpenFile();
};

template <typename... Args>
void Logger::Log(PriorityEnum aPriority, std::string aFile, std::string aFunction, int aLine, std::string format,
                 Args&&... vs)
{
	if (aPriority < mLogLevel)
	{
		return;
	}

	// собираем сообщение
	const std::string messageText = Utils::Format(format.c_str(), vs...);
	// получаем короткое имя файла
	std::string fileString = Utils::GetFileNameWithExtension(aFile);

	// лочимся
	std::lock_guard<std::mutex> lock(mMutex);
	// собираем запись лога
	std::shared_ptr<LogEntry> logEntry = std::make_shared<LogEntry>(std::chrono::system_clock::now(), aPriority,
	                                                                messageText, fileString, aFunction, aLine);
	// фильтруем идущие друг за другом одинаковые сообщения
	bool repeatedLogEntry = false, replaceLogEntry = true;
	if (logEntry->Message.compare(mLastLogEntry.Message) == 0)
	{
		++mLastLogEntry.Counter;
		mLastLogEntry.Time = logEntry->Time;
		replaceLogEntry = false;
		repeatedLogEntry = true;
	}
	else if (mLastLogEntry.Counter > 0)
	{
		// пишем то что повторялось один разик с указанием кол-ва повторов
		ProcessLog(std::make_shared<LogEntry>(mLastLogEntry));
		mLastLogEntry.Counter = 0;
	}
	if (!repeatedLogEntry)
	{
		// пишем то что ещё не повторялось
		ProcessLog(logEntry);
	}
	if (replaceLogEntry)
	{
		mLastLogEntry = *logEntry;
	}
}

#ifdef _WIN32

#define LOGGER_LOG(priority, format, ...) \
    Logger::Instance().Log(priority, __FILE__, __func__, __LINE__, format, __VA_ARGS__)

#define LOGGER_NULLPTR(name) \
    Logger::Instance().Log(PriorityEnum::Error, __FILE__, __func__, __LINE__, "%s(%d): %s is nullptr", name)

#elif __linux__

#define LOGGER_LOG(priority, format, ...) \
    Logger::Instance().Log(priority, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)

//Logger::template LogStatic<args>(priority, "", "", "", format)

#define LOGGER_NULLPTR(name) \
    Logger::Instance().Log(PriorityEnum::Error, __FILE__, __func__, __LINE__, "%s(%d): %s is nullptr", name)

#endif

} // namespace VMM

#endif // SUBPRJ_COMMON_BASE_LOGGER_H_
