#ifndef RACE_H_
#define RACE_H_

#include <memory>
#include <functional>
#include <mutex>

#include "Logger.h"

#include "../BlackBox/BlackBox.h"
#include "./Settings.h"
#include "./Defines.h"


namespace Fatracing {

struct RaceStruct {
    unsigned int Seconds;

    uint64_t BlueScore;
    uint64_t RedScore;

    uint64_t BlueRPM;
    uint64_t RedRPM;
};

class Race {
public:
    typedef std::function<void(RaceStruct)> RaceCallback;

private:
    std::shared_ptr<BlackBox> mBlackBox = nullptr;
    SettingsStruct mSettings;
    RaceCallback mRaceCallback;

    std::mutex mRaceStateMutex;
    RaceStruct mCurrentRaceState;

public:
    Race(SettingsStruct& aSettings, RaceCallback aRaceCallback);

    void Start();
    void Stop();

private:
    void TimerTick();
    void BlackBoxCallback(RacersEnum aRacer);
};

}

#endif // RACE_H_
