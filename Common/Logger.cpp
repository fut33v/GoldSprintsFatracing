// Copyright 2018

#include "Logger.h"

namespace Fatracing {
Logger::Logger() {
	if (mWriteToFileEnabled) {
		OpenFile();
	}

	mLogLevel = PriorityEnum::Trace;
}

Logger& Logger::Instance() {
	static Logger instance;
	return instance;
}

void Logger::Dump() {
	if (mFileStream.is_open()) {
		mFileStream.flush();
	}
}

void Logger::AddCallback(const std::string& aCallbackName, LoggerCallback aCallback) {
	if (aCallback == nullptr) {
		return;
	}
	std::lock_guard<std::mutex> lock(mMutex);
	mLoggerCallbacks[aCallbackName] = aCallback;
}

void Logger::RemoveCallback(const std::string& aCallbackName) {
    std::lock_guard<std::mutex> lock(mMutex);
    mLoggerCallbacks.erase(aCallbackName);    
}

void Logger::ClearCallbacks() {
	std::lock_guard<std::mutex> lock(mMutex);
	mLoggerCallbacks.clear();
}

void Logger::SetWriteToFileEnabled(bool aWriteToFileEnabled) {
	mWriteToFileEnabled = aWriteToFileEnabled;
}

void Logger::SetWriteToConsoleEnabled(bool aWriteToConsoleEnabled) {
	mWriteToConsoleEnabled = aWriteToConsoleEnabled;
}

void Logger::SetLogLevel(PriorityEnum aPriority) {
	std::lock_guard<std::mutex> lock(mMutex);
	mLogLevel = aPriority;
}

std::string Logger::PriorityToString(PriorityEnum aPriority) {
	return Instance().PriorityString[static_cast<size_t>(aPriority)];
}

Logger::~Logger() {
	if (mFileStream.is_open()) {
		mFileStream.flush();
		mFileStream.close();
	}
}

void Logger::ProcessLog(std::shared_ptr<LogEntry> logEntry) {
	// пишем лог в файл
	if (mWriteToFileEnabled) {
        if (mFileStream.is_open()) {
#ifdef _WIN32
            mFileStream << logEntry->Print() << std::endl;
#elif __linux__
            mFileStream << logEntry->Print() << std::endl;
#endif
		}
	}
	// выводим лог в GUI и другие компоненты
	for (auto& x : mLoggerCallbacks) {
		x.second(logEntry);
	}
	// выводим в консоль
	if (mWriteToConsoleEnabled) {
		std::cout << logEntry->Print() << std::endl;
	}
}

bool Logger::OpenFile() {
	if (mFileStream.is_open()) {
		return false;
	}
	LogFilePath = Utils::FormatFileNameYearMonthDayHourSecond(std::chrono::system_clock::now()) + LogFileExtension;
	mFileStream.open(LogFilePath, std::ofstream::out);
	return true;
}
} // namespace Fatracing
