// Copyright 2018

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <functional>

namespace Fatracing
{

    class FileSaver
    {
        // Путь к файлу
        std::string mFilePath;
        // Стрим
        std::ofstream mFile;

    public:
        FileSaver();
        ~FileSaver();

        // Инициализация
        bool InitFileSaver(const std::string& aFilePath, bool aAppend = false);
        // Запись строки в файл
        bool WriteData(const std::string& aData);
    };
}

