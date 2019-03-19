#include <functional>

#include "./Race.h"


namespace Fatracing {

Race::Race(SettingsStruct &aSettings, Race::RaceCallback aRaceCallback) {
    mSettings = aSettings;
    mRaceCallback = aRaceCallback;
    Clear();
}

Race::~Race() {
    mStopThread = true;
    if (mThread.joinable()) {
        mThread.join();
    }
}

void Race::Init() {
    if (mBlackBox) {
        mBlackBox.reset();
    }
    mBlackBox = std::make_shared<BlackBox>();
    SerialPortSettings ss;
    ss.PortName = mSettings.PortName;
    ss.PortOnly = true;
    mBlackBox->Init(ss);
    mBlackBox->SetCallback(std::bind(&Race::BlackBoxCallback, this, std::placeholders::_1));
}

void Race::Start() {
    Clear();
    if (mThread.joinable()) {
        mThread.join();
    }
    mThread = std::thread([&](){
        int i = mSettings.RaceTimeSeconds;
        while (i >= 0) {
        //for (; i >= 0; --i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (mStopThread) {
                break;
            }
            TimerTick();
            i--;
        }
    });
}

void Race::Clear() {
    std::unique_lock<std::mutex> lock(mRaceStateMutex);
    mCurrentRaceState.Seconds = mSettings.RaceTimeSeconds;
    mCurrentRaceState.BlueScore = 0;
    mCurrentRaceState.RedScore = 0;
    mCurrentRaceState.BlueRPM = 0;
    mCurrentRaceState.RedRPM = 0;
    mCurrentRaceState.Finish = false;

    if (mRaceCallback) {
        mRaceCallback(mCurrentRaceState);
    }
}


void Race::TimerTick() {
    std::unique_lock<std::mutex> lock(mRaceStateMutex);
    if (mCurrentRaceState.Seconds > 0) {
        mCurrentRaceState.Seconds--;
    } else {
        mCurrentRaceState.Finish = true;
    }

    mCurrentRaceState.BlueRPM = (mCurrentRaceState.BlueScore - mCurrentRaceState.PrevBlueScore) * 60;
    mCurrentRaceState.RedRPM = (mCurrentRaceState.RedScore - mCurrentRaceState.PrevRedScore) * 60;

    mCurrentRaceState.PrevBlueScore = mCurrentRaceState.BlueScore;
    mCurrentRaceState.PrevRedScore = mCurrentRaceState.RedScore;

    RaceStruct r = mCurrentRaceState;
    lock.unlock();

    if (mRaceCallback) {
        mRaceCallback(r);
    }
}

void Race::BlackBoxCallback(RacersEnum aRacer) {
    std::unique_lock<std::mutex> lock(mRaceStateMutex);

    if (!mCurrentRaceState.Finish) {
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
    }

    if (mCurrentRaceState.BlueScore > mCurrentRaceState.RedScore) {
        mCurrentRaceState.Leader = RacersEnum::BLUE;
        mCurrentRaceState.Diff = mCurrentRaceState.BlueScore - mCurrentRaceState.RedScore;
    } else {
        mCurrentRaceState.Leader = RacersEnum::RED;
        mCurrentRaceState.Diff = mCurrentRaceState.RedScore - mCurrentRaceState.BlueScore;
    }
    RaceStruct r = mCurrentRaceState;
    lock.unlock();

    if (mRaceCallback) {
        mRaceCallback(r);
    }
}

} // namespace Fatracing
