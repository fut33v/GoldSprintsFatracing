// Copyright 2018

#ifndef VISM_MAIN_SUBPRJ_COMMON_SIGNALCALLBACK_H_ 
#define VISM_MAIN_SUBPRJ_COMMON_SIGNALCALLBACK_H_ 

#include <functional>


namespace VMM
{
typedef std::function<void()> SignalCallback;
typedef std::function<void(bool)> SignalBoolCallback;

inline void InvokeSignal(SignalCallback signalCallback)
{
	if (signalCallback)
	{
		signalCallback();
	}
}
inline void InvokeSignalBool(SignalBoolCallback aSignalBoolCallback, bool aBool)
{
	if (aSignalBoolCallback)
	{
		aSignalBoolCallback(aBool);
	}
}
}

#endif // AFA41_10_MAIN_SUBPRJ_COMMON_SIGNALCALLBACK_H_ 
