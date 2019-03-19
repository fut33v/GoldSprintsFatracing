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
    int Seconds;

    uint64_t BlueScore = 0;
    uint64_t RedScore = 0;

    uint64_t BlueRPM = 0;
    uint64_t RedRPM = 0;

    bool Finish=false;

    RacersEnum Leader;
    uint64_t Diff;

    uint64_t PrevBlueScore = 0;
    uint64_t PrevRedScore = 0;
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

    std::thread mThread;
    std::atomic<bool> mStopThread{false};

public:
    Race(SettingsStruct& aSettings, RaceCallback aRaceCallback);
    ~Race();

    void Init();
    void Start();
    void Clear();

private:
    void TimerTick();
    void BlackBoxCallback(RacersEnum aRacer);
};

}

#endif // RACE_H_
