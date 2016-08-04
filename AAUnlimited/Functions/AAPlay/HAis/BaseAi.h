#pragma once

#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\IllusionList.h"
#include "External\ExternalClasses\HClasses\HGUIButton.h"
#include "General\Util.h"

class BaseAi {
public:
	virtual void Initialize(ExtClass::HInfo* info) = 0;
	virtual void Tick(ExtClass::HInfo* hinfo) = 0;

	//sets timer channel to a random value between min and max
	inline void StartTimer(int channel);
	inline void StartTimerRandom(int channel, float min, float max);
	inline bool TimerPassed(int channel);

	inline void SetRepeatParams(int channel, float repeatChance, float decay);
	inline bool WantRepeat(int channel);

	inline void SetSpeedChangeLinear(float curr, float target, float changeOver);
	inline void SetSpeedChangeFluctuate(float curr, float middle, float maxDiff);
	inline float GetSpeed();

	inline void DisableAllButtons(ExtClass::HInfo* info);
private:
	static const unsigned int N_CHANNELS = 10;
	General::PassiveTimer m_timers[N_CHANNELS];
	float m_waitTimes[N_CHANNELS];

	float m_repeatChance[N_CHANNELS];
	float m_alpha[N_CHANNELS];

	int m_speedMode;
	General::PassiveTimer m_speedTimer;

	struct {
		float start;
		float end;
		float changeOver;
	} m_speedLinear;
	struct {
		float min;
		float max;
	} m_speedFluctuate;
	
	
};

//timerops
inline void BaseAi::StartTimer(int channel) {
	m_timers[channel].Start();
}

//sets timer channel to a random value between min and max
inline void BaseAi::StartTimerRandom(int channel, float min, float max) {
	m_waitTimes[channel] = General::GetRandomFloat(min, max);
	m_timers[channel].Start();
}

inline bool BaseAi::TimerPassed(int channel) {
	return m_timers[channel].GetTime() >= m_waitTimes[channel];
}

//repeat ops
inline void BaseAi::SetRepeatParams(int channel, float repeatChance, float decay) {
	m_repeatChance[channel] = repeatChance;
	m_alpha[channel] = decay;
}

inline bool BaseAi::WantRepeat(int channel) {
	float randVal = rand() / (float)RAND_MAX;
	bool wantRepeat = randVal <= m_repeatChance[channel];
	m_repeatChance[channel] *= m_alpha[channel];
	return wantRepeat;
}

//speed ops
inline void BaseAi::SetSpeedChangeLinear(float start, float target, float changeOver) {
	m_speedLinear.start = start;
	m_speedLinear.end = target;
	m_speedLinear.changeOver = changeOver;
	m_speedMode = 0;
	m_speedTimer.Start();
}
inline void BaseAi::SetSpeedChangeFluctuate(float curr, float min, float max) {
	m_speedFluctuate.min = min;
	m_speedFluctuate.max = max;
	
	m_speedLinear.start = curr;
	m_speedLinear.end = curr;
	m_speedLinear.changeOver = 0;

	m_speedMode = 1;
	m_speedTimer.Start();
}
inline float BaseAi::GetSpeed() {
	switch (m_speedMode) {
	case 0: {
		auto& d = m_speedLinear;
		float passed = (float)m_speedTimer.GetTime();
		if (passed > d.changeOver) return d.end;
		return d.start + (d.end - d.start) * (passed / d.changeOver);
		break; }
	case 1: {
		auto& d = m_speedFluctuate;
		auto& ld = m_speedLinear;
		float passed = (float)m_speedTimer.GetTime();
		if (passed >= ld.changeOver) {
			//set new linear progression
			ld.start = ld.end;
			ld.end = General::GetRandomFloat(d.min, d.max);
			ld.changeOver = abs((ld.end - ld.start) / (General::GetRandomFloat(0.01f, 0.3f)));
			m_speedTimer.Start();
			return ld.start;
		}
		else {
			//return linear progression
			if (passed >= ld.changeOver) return ld.end;
			return ld.start + (ld.end - ld.start) * (passed / ld.changeOver);
		}
		break; }
	default:
		break;
	}
	return 0;
}


inline void BaseAi::DisableAllButtons(ExtClass::HInfo* info) {
	using namespace ExtClass;
	IllusionList* start = info->m_ptrAllButtons->buttonList;
	IllusionList* it = start;
	int nButtons = (int)it->data;
	it = it->next;
	do {
		HGUIButton* btn = (HGUIButton*)it->data;
		btn->m_bActive = FALSE;
		it = it->next;
	} while (it != start);
}