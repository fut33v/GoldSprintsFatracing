#include <functional>

#include "./Race.h"


namespace Fatracing {

Race::Race(SettingsStruct &aSettings, Race::RaceCallback aRaceCallback) {
    mSettings = aSettings;
    //        if (aRaceCallback == nullptr) {
    //            throw std::exception();
    //        }
    mRaceCallback = aRaceCallback;

    mCurrentRaceState.BlueScore = 0;
    mCurrentRaceState.RedScore = 0;
}

void Race::Start() {
    if (mBlackBox) {
        mBlackBox.reset();
    }
    mBlackBox = std::make_shared<BlackBox>();
    SerialPortSettings ss;
    ss.PortName = mSettings.PortName;
    ss.PortOnly = true;
    mBlackBox->Init(ss);
    mBlackBox->SetCallback(std::bind(&Race::BlackBoxCallback, this, std::placeholders::_1));


    // start timer here

}

void Race::Stop() {

}

void Race::TimerTick() {
    // get current race time
    // get race counters and put it to race struct

    RaceStruct r;

    if (mRaceCallback) {
        mRaceCallback(r);
    }
}

void Race::BlackBoxCallback(RacersEnum aRacer) {
    std::unique_lock<std::mutex> lock(mRaceStateMutex);
    switch (aRacer) {
    case RacersEnum::BLUE: {
        mCurrentRaceState.BlueScore += 1;
        break;
    }
    case RacersEnum::RED: {
        mCurrentRaceState.RedScore += 1;
        break;
    }
    }

    RaceStruct r = mCurrentRaceState;
    lock.unlock();

    if (mRaceCallback) {
        mRaceCallback(r);
    }
}

} // namespace Fatracing
