#pragma once

#include "CameraDefines.h"
#include "CommonStructures.h"
#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <array>

namespace VMM
{

enum CameraType : uint8_t
{
	TV_Gige
};


//! структура, содержащая изображение
struct Image : public Data
{
    int camera = 0;
    unsigned long width = 0u;
    unsigned long height = 0u;
    unsigned long blackLevel = 0u;
    uint32_t exposureTime = 0;
	uint64_t frameNumber = 0;	
	uint16_t bitDepth = 12;
	uint16_t bytesPerPixel = 2;
    float gainDb = 0.0;
    ColorType colorType = ColorType::Monochrome;

    long long cameraTime = 0;
    
    std::array<char, 20> serialNumber;

    void SetData(unsigned char* aData, size_t aDataLength)
    {
        data.assign(aData, aData + aDataLength);
    }
};

//! обертка для содержания указателя на структуру
class ImageHandlerIf
{
public:
    virtual void HandleImage(std::shared_ptr<Image> aImage) = 0;
};

//! Интерфейс для вывода сообщений от камеры
class ICameraOutput : public ImageHandlerIf
{
public:
    //! Передача события от камеры на обработку
    virtual void HandleError(ErrorType errType, const std::string& error) = 0;
};

// interface for CamearaDevices, that wraps mCamera API
class ICameraAPI
{
public:
    //! Деструктор
    virtual ~ICameraAPI()
    {
    }

    //! Инициализация передачи данных и драйверов камеры
    virtual bool Init(const CameraInfo& cameraInfo, ICameraOutput* handler) = 0;

    //! Запустить видеопоток
    virtual bool StartCapture() = 0;

    //! Остановить видеопоток
    virtual bool StopCapture() = 0;

	//! установить параметры
    virtual bool SetParameter(const std::string& name, const CameraParameter& paramType) = 0;
};
}
