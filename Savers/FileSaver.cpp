// Copyright 2018

#include "FileSaver.h"
#include "Logger.h"

namespace VMM
{
    FileSaver::FileSaver()
    {
    }

    FileSaver::~FileSaver()
    {
        mFile.close();
    }

    bool FileSaver::InitFileSaver(const std::string& aFilePath, bool aAppend)
    {
        mFilePath = aFilePath;

        std::ios_base::openmode mode = std::ios::out;
        if (aAppend)
        {
            mode |= std::ios::app;
        }
        mFile.open(mFilePath, mode);
        return mFile.is_open();
    }

    bool FileSaver::WriteData(const std::string& aData)
    {
        if (mFile.is_open())
        {
            mFile << aData << std::endl;
            return true;
        }
        return false;
    }    
}
