#pragma once
#include "AudioManager.h"

class ClickSound
{
public:

	static ClickSound& Instance()
	{
		static ClickSound instance;
		return instance;
	}

	void Init(bool* isEnabled, float* volume)
	{
		this->isEnabled = isEnabled;
		this->volume = volume;
	}
	void PlayPopSound() const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\pop.wav", *volume);
	}
	void PlayClickSound() const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\click.wav", *volume);
	}
	void PlayOnSound() const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\on.wav", *volume);
	}
	void PlayOffSound()const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\off.wav", *volume);
	}
	void PlaySaveSound()const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\save.wav", *volume);
	}
	void PlayExitSound()const
	{
		if (*isEnabled) AudioManager::Instance().playSound("menu\\exit.wav", *volume);
	}
	static void PlayIntroSound()
	{
		AudioManager::Instance().playSound("intro.wav", 0.3f);
	}
private:
	bool* isEnabled;
	float* volume;
};
