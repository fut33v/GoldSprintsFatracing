#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace VMM
{
    //! Тип ошибки
    enum class ErrorType
    {
		Trace,
		Info,
		Warning,
        Error
    };

    enum class ColorType : unsigned short
    {
        Monochrome = 1,
		BayerGR8,
		BayerBG8
    };
    
    struct CameraParameter
    {
        std::string type;
        std::string value;
    };

	//! for AddCamera	
	struct GeneralCameraParams 
    {		
        unsigned int cameraId;

        std::string cameraIdentificator;

        std::string ipAddress;
		std::string libraryName;
        std::string savePath;
	};

    typedef std::unordered_map<std::string, CameraParameter> SettingsMap;

	//! load by CameraManager
    struct CameraInfo 
    {
		GeneralCameraParams general;
        SettingsMap settings;
    };	    
};
