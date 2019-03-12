#ifndef RACE_H_
#define RACE_H_

#include <memory>
#include <functional>

#include "Logger.h"

#include "../BlackBox/BlackBox.h"
#include "Settings.h"


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
    SettingsStruct& mSettings;
    RaceCallback mRaceCallback;

public:
    Race(SettingsStruct& aSettings, RaceCallback aRaceCallback) {
        mSettings = aSettings;
//        if (aRaceCallback == nullptr) {
//            throw std::exception();
//        }
        mRaceCallback = aRaceCallback;
    }

    void Start() {
        if (mBlackBox) {
           mBlackBox.reset();
        }
        mBlackBox = std::make_shared<BlackBox>();
        SerialPortSettings ss;
        ss.PortName = mSettings.PortName;
        ss.PortOnly = true;
        mBlackBox->Init(ss);

        // start timer here

    }


    void Stop() {

    }

private:
    void TimerTick() {
        // get current race time
        // get race counters and put it to race struct

        RaceStruct r;

        if (mRaceCallback) {
            mRaceCallback(r);
        }
    }
};

}

#endif // RACE_H_
