// Copyright 2018

#pragma once

#include <vector>
#include <chrono>

enum class FileType : unsigned char
{
    Log,
    Identify,
    Passport,
    FlyData,
    Climate
};

typedef std::chrono::time_point<std::chrono::system_clock> SystemTime;

struct Data
{
    SystemTime time = std::chrono::system_clock::now();
    std::vector<unsigned char> data;
};