#pragma once
//------------------------------------
// By Jishnu Girish
//------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// This Audio Engine is based of the How to Make an Audio Engine GDC Talk given by Guy Somberg
// http://gdcvault.com/play/1022060/How-to-Write-an-Audio
// ------------------------------------------------------------------------------------------------------------------------


#include "fmod.h"
#include "fmod_dsp.h"
#include "fmod_dsp_effects.h"
#include "fmod_errors.h"
#include "fmod_common.h"
#include "fmod_codec.h"
#include "fmod_output.h"
#include "fmod_studio.hpp"
#include <map>
#include <vector>
#include <math.h>
#include <iostream>
#include <memory>

using namespace FMOD;
using namespace std;

// --------------------------------------------------------
// Vectors used in audio positioning
// --------------------------------------------------------
struct AudioVector3 {
	float x;
	float y;
	float z;
	AudioVector3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

// ----------------------------------------------------------
// Structure that defines reverb properties
// ----------------------------------------------------------

struct ReverbProperties {
	float DecayTime;
	float EarlyDelay;
	float LateDelay;
	float HFReference;
	float HFDecayRatio;
	float Diffusion;
	float Density;
	float LowShelfFrequency;
	float LowShelfGain;
	float HighCut;
	float EarlyLateMix;
	float WetLevel;
	ReverbProperties(float _DecayTime, float _EarlyDelay, float _LateDelay, float _HFReference, float _HFDecayRatio,
		float _Diffusion, float _Density, float _LowShelfFrequency, float _LowShelfGain, float _HighCut, float _EarlyLateMix,
		float _WetLevel)
	{
		 DecayTime = _DecayTime;
		 EarlyDelay = _EarlyDelay;
		 LateDelay =_LateDelay;
		 HFReference = _HFReference;
		 HFDecayRatio =_HFDecayRatio;
		 Diffusion =_Diffusion;
		 Density = _Density;
		 LowShelfFrequency =_LowShelfFrequency;
		 LowShelfGain = _LowShelfGain;
		 HighCut = _HighCut;
		 EarlyLateMix = _EarlyLateMix;
		 WetLevel = _WetLevel;
	}
};
static const ReverbProperties ENVIRONMENT_UNDERWATER		= { 1500,    7,  11, 5000,  10, 100, 100, 250, 0,   500,  92,   7.0f };
static const ReverbProperties ENVIRONMENT_GENERIC			= { 1500,    7,  11, 5000,  83, 100, 100, 250, 0, 14500,  96,  -8.0f };
static const ReverbProperties ENVIRONMENT_PADDEDCELL		= { 170,    1,   2, 5000,  10, 100, 100, 250, 0,   160,  84,  -7.8f };
static const ReverbProperties ENVIRONMENT_ROOM				= { 400,    2,   3, 5000,  83, 100, 100, 250, 0,  6050,  88,  -9.4f };
static const ReverbProperties ENVIRONMENT_BATHROOM			= { 1500,    7,  11, 5000,  54, 100,  60, 250, 0,  2900,  83,   0.5f };
static const ReverbProperties ENVIRONMENT_LIVINGROOM		= { 500,    3,   4, 5000,  10, 100, 100, 250, 0,   160,  58, -19.0f };
static const ReverbProperties ENVIRONMENT_STONEROOM			= { 2300,   12,  17, 5000,  64, 100, 100, 250, 0,  7800,  71,  -8.5f };
static const ReverbProperties ENVIRONMENT_AUDITORIUM		= { 4300,   20,  30, 5000,  59, 100, 100, 250, 0,  5850,  64, -11.7f };
static const ReverbProperties ENVIRONMENT_CONCERTHALL		= { 3900,   20,  29, 5000,  70, 100, 100, 250, 0,  5650,  80,  -9.8f };
static const ReverbProperties ENVIRONMENT_CAVE				= { 2900,   15,  22, 5000, 100, 100, 100, 250, 0, 20000,  59, -11.3f };
static const ReverbProperties ENVIRONMENT_ARENA				= { 7200,   20,  30, 5000,  33, 100, 100, 250, 0,  4500,  80,  -9.6f };
static const ReverbProperties ENVIRONMENT_HANGER			= { 10000,   20,  30, 5000,  23, 100, 100, 250, 0,  3400,  72,  -7.4f };
static const ReverbProperties ENVIRONMENT_CARPETTEDHALLWAY	= { 300,    2,  30, 5000,  10, 100, 100, 250, 0,   500,  56, -24.0f };
static const ReverbProperties ENVIRONMENT_HALLWAY			= { 1500,    7,  11, 5000,  59, 100, 100, 250, 0,  7800,  87,  -5.5f };
static const ReverbProperties ENVIRONMENT_STONECORRIDOR		= { 270,   13,  20, 5000,  79, 100, 100, 250, 0,  9000,  86,  -6.0f };
static const ReverbProperties ENVIRONMENT_ALLEY				= { 1500,    7,  11, 5000,  86, 100, 100, 250, 0,  8300,  80,  -9.8f };
static const ReverbProperties ENVIRONMENT_FOREST			= { 1500,  162,  88, 5000,  54,  79, 100, 250, 0,   760,  94, -12.3f };
static const ReverbProperties ENVIRONMENT_CITY				= { 1500,    7,  11, 5000,  67,  50, 100, 250, 0,  4050,  66, -26.0f };
static const ReverbProperties ENVIRONMENT_MOUNTAINS			= { 1500,  300, 100, 5000,  21,  27, 100, 250, 0,  1220,  82, -24.0f };
static const ReverbProperties ENVIRONMENT_QUARRY			= { 1500,   61,  25, 5000,  83, 100, 100, 250, 0,  3400, 100,  -5.0f };
static const ReverbProperties ENVIRONMENT_PLAIN				= { 1500,  179, 100, 5000,  50,  21, 100, 250, 0,  1670,  65, -28.0f };
static const ReverbProperties ENVIRONMENT_PARKINGLOT		= { 1700,    8,  12, 5000, 100, 100, 100, 250, 0, 20000,  56, -19.5f };
static const ReverbProperties ENVIRONMENT_SEWERPIPE			= { 2800,   14,  21, 5000,  14,  80,  60, 250, 0,  3400,  66,   1.2f };
static const ReverbProperties ENVIRONMENT_REVERB_OFF		= { 1000,    7,  11, 5000, 100, 100, 100, 250, 0,    20,  96, -80.0f };

// ---------------------------------------------------------
// Contains all the calls to the FMOD system
// Engine calls this struct to start stop and update FMOD
// ---------------------------------------------------------
struct Implementation {
private:
	static Implementation* instance;
	Implementation();
public:
	static Implementation* Instance()
	{
		if (!instance)
			instance = new Implementation();
		return instance;
	}
	~Implementation();

	void Update();
	FMOD::System* audioSystem;
	FMOD::Studio::System* mpStudioSystem;
	FMOD::Reverb3D* reverb;

	int nextChannelId;											// Id of the next channel a sound will play in

	typedef map < string, Sound* > SoundMap;					// Map os all sounds tracks in the project and their file
	typedef map < int, Channel* > ChannelMap;					// Map of all channels ind its ID
	typedef map < string, Studio::EventInstance*> EventMaps;	// Map of all FMOD Studio events
	typedef map < string, Studio::Bank* > BankMaps;				// Map of all fmod studio banks
	typedef map < string, Channel*> SoundChannelMap;			// Map of all sounds to channels they play in

	SoundMap audioSounds;										// Sound cache
	ChannelMap audioChannels;									// ChannelID cache
	BankMaps audioBanks;										// Audio bank cache
	EventMaps audioEvents;										// Events cache
	SoundChannelMap audioSoundChannels;							// reference of sounds and channels
};

class AudioEngine
{
private:
	static AudioEngine* instance;
	AudioEngine();
public:

	static AudioEngine* Instance()
	{
		return instance;
	}
	~AudioEngine() {}

	static void Init();
	static void Update();
	static void ShutDown();
	static int ErrorCheck(FMOD_RESULT result);

	void LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
	void LoadEvents(const string& eventName);
	void LoadSound(const string& soundName,bool _3D=true,bool bLooping = false,bool bStream = false);
	void UnLoadSound(const string& soundName);

	void Set3dListenerAndOrientation(const AudioVector3& position,const AudioVector3& look, const AudioVector3& up);
	int PlaySounds(const string& soundName, const AudioVector3& position = {0,0,0}, float volume = 0.0f, float fadeInTime = 0.0f);
	void PlayEvents(const string& eventName);
	void StopChannel(int channelId,float fadeOutTime = 0.0f);
	void StopEvent(const string& eventName, bool immediate = false);

	void GetEventParameter(const string& eventName,const string& eventParameter,float* parameter);
	void SetEventParameter(const string& eventName, const string& eventParameter, float value);

	void StopAllChannels();
	void SetChannel3DPosition(int channelId,const AudioVector3& position);
	void SetChannelVolume(int channelId, float volume);
	bool IsPlaying(int channleId) const;
	bool IsEventPlaying(const string& eventName)const;
	float DecibalToVolume(float dBVolume);
	float VolumeToDecibal(float volume);
	FMOD_VECTOR VectorToFmod(const AudioVector3& vposition);

	void MoveChannel3DPosition(int channelId, const AudioVector3& velocity);
	void FadeOutChannel(int channelId, float fadeOutTime = 0.0f, float fadeOutVolume = 0.0f);
	void FadeInChannel(int channelId, float fadeInTime = 0.0f, float fadeInVolume = 1.0f);

	void SetPitch(int channelId, float pitch = 1.0f);
	float GetPitch(int channelId);
	void SetFrequency(int channelId, float frequency);
	float GetFrequency(int channelId);

	void SetLowPassGain(int channelId, float gain = 1.0f);
	float GetLowPassGain(int channelId);

	void ActivateReverb(bool active);
	bool GetReverbState() const;
	FMOD_REVERB_PROPERTIES SetReverbProperties(const ReverbProperties& reverbProp) const;
	void SetEnvironmentReverb(const FMOD_REVERB_PROPERTIES reverbProperties,const AudioVector3& position,float minDist,float maxDist);
};

