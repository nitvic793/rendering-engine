//------------------------------------
// By Jishnu Girish
//------------------------------------
#include "AudioEngine.h"

Implementation *Implementation::instance = 0;
AudioEngine *AudioEngine::instance = 0;
//--------------------------------------------------------------
// Initialize the FMOD system
//--------------------------------------------------------------
Implementation::Implementation()
{
	mpStudioSystem = nullptr;
	AudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
	AudioEngine::ErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL)); // Support 32 channels, Allows live update with FMOD studio

	audioSystem = nullptr;
	AudioEngine::ErrorCheck(mpStudioSystem->getLowLevelSystem(&audioSystem));

	reverb = nullptr;
	nextChannelId = 0;
}

//--------------------------------------------------------------
// Release FMOD resources
//--------------------------------------------------------------
Implementation::~Implementation()
{
	AudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
	AudioEngine::ErrorCheck(mpStudioSystem->release());
}

//--------------------------------------------------------------
// Update the FMOD system
//--------------------------------------------------------------
void Implementation::Update()
{
	vector<ChannelMap::iterator> stoppedChannels;
	for (auto it = audioChannels.begin(), itEnd = audioChannels.end(); it != itEnd; it++)
	{
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);

		if (!isPlaying)
		{
			stoppedChannels.push_back(it);			// list of all stopped channels
		}
	}

	for (auto &it : stoppedChannels)
	{
		audioChannels.erase(it);					// Destroy stopped channels
	}

	vector<SoundChannelMap::iterator> stoppedChannelsSounds;
	for (auto it = audioSoundChannels.begin(), itEnd = audioSoundChannels.end(); it != itEnd; it++)
	{
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);

		if (!isPlaying)
		{
			stoppedChannelsSounds.push_back(it);			// list of all stopped channels
		}
	}

	for (auto &it : stoppedChannelsSounds)
	{
		audioSoundChannels.erase(it);					// Destroy stopped channels
	}

	AudioEngine::ErrorCheck(mpStudioSystem->update());
}

//--------------------------------------------------------------
// Check for FMOD errors
//--------------------------------------------------------------
int AudioEngine::ErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		cout << "\n FMOD ERROR " << result;
		return 1;
	}
	// cout << "FMOD all good" << endl;
	return 0;
}

//--------------------------------------------------------------
// Initialize the engine
//--------------------------------------------------------------
void AudioEngine::Init()
{
	
}

//--------------------------------------------------------------
// Update the engine properties
//--------------------------------------------------------------
void AudioEngine::Update()
{
	Implementation::Instance()->Update();
}

//--------------------------------------------------------------
// Load a sound track and store in the queue for playback
//--------------------------------------------------------------
void AudioEngine::LoadSound(const string& soundName, bool _3D, bool looping, bool stream)
{
	auto foundIt = Implementation::Instance()->audioSounds.find(soundName);			// Find sound  from the list of loaded files (cache)
	if (foundIt != Implementation::Instance()->audioSounds.end())
		return;

	// Decides what mode the sound should be loaded as
	FMOD_MODE mode = FMOD_DEFAULT;
	mode |= _3D ? FMOD_3D : FMOD_2D;
	mode |= looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
	mode |= FMOD_INIT_CHANNEL_LOWPASS;											// enable low pass filtering

	FMOD::Sound * currentSound = nullptr;
	int error = AudioEngine::ErrorCheck(Implementation::Instance()->audioSystem->createSound(soundName.c_str(), mode, nullptr, &currentSound));
	if (error == 1)
	{
		std::cout<<"\nCould not load sound!! " << soundName.c_str();
		return;
	}
	// If sound loaded correctly
	// add it to the queue of all sounds (cache)
	if (currentSound)
	{
		Implementation::Instance()->audioSounds[soundName] = currentSound;
	}
}

//--------------------------------------------------------------
// Unload sounds in the cache
//--------------------------------------------------------------
void AudioEngine::UnLoadSound(const string& soundName)
{
	auto foundIt = Implementation::Instance()->audioSounds.find(soundName);			// Find sound  from the list of loaded files (cache)
	if (foundIt != Implementation::Instance()->audioSounds.end())
		return;

	// Clear sound after use
	AudioEngine::ErrorCheck(foundIt->second->release());
	Implementation::Instance()->audioSounds.erase(foundIt);
}

//--------------------------------------------------------------
// Set the listener position and orientation
//--------------------------------------------------------------
void AudioEngine::Set3dListenerAndOrientation(const AudioVector3 & position, const AudioVector3 & look, const AudioVector3 & up)
{
	FMOD_VECTOR pos = VectorToFmod(position);
	FMOD_VECTOR forward = VectorToFmod(look);
	FMOD_VECTOR updir = VectorToFmod(up);
	FMOD_VECTOR velocity = VectorToFmod(AudioVector3{ 0,0,0 });
	AudioEngine::ErrorCheck(Implementation::Instance()->audioSystem->set3DListenerAttributes(0,&pos,&velocity,&forward,&updir));
}

// ------------------------------------------------------------
// Play sounds that have been loaded correctly
// create channels for the sound to play
// sound is initially paused to avoid the sound popping in abruptly
// returns the channel ID of playing sound
// ------------------------------------------------------------
int AudioEngine::PlaySounds(const string& soundName, const AudioVector3& position, float volume, float fadeInTime)
{
	int nxtChannelId = Implementation::Instance()->nextChannelId++;
	auto foundIt = Implementation::Instance()->audioSounds.find(soundName);			// Find sound  from the list of loaded files
	if (foundIt == Implementation::Instance()->audioSounds.end())					// If sond not found then try and load sound
	{
		LoadSound(soundName);
		foundIt = Implementation::Instance()->audioSounds.find(soundName);
		if (foundIt == Implementation::Instance()->audioSounds.end())				// Coundn't load sound hence cannot play osund
		{
			return nxtChannelId;
		}
	}

	Channel * currentChannel = nullptr;											// New channel to play sound in
	
	AudioEngine::ErrorCheck(Implementation::Instance()->audioSystem->playSound(foundIt->second, nullptr, true, &currentChannel));	// pause on startup to avoid cutting

	if (currentChannel)
	{
		FMOD_MODE currentMode;
		foundIt->second->getMode(&currentMode);

		if (currentMode & FMOD_3D)																	// if sound is in 3D mode
		{
			FMOD_VECTOR currPosition = VectorToFmod(position);										// set 3D position
			AudioEngine::ErrorCheck(currentChannel->set3DAttributes(&currPosition, nullptr));		// set 3D channel attributes
		}
		Implementation::Instance()->audioChannels[nxtChannelId] = currentChannel;						// Update Channel cache
		this->FadeInChannel(nxtChannelId, fadeInTime, DecibalToVolume(volume));						// Fade the channel into a certain volume level
		Implementation::Instance()->audioSoundChannels[soundName] = currentChannel;						// Map sound name to channel
		
		AudioEngine::ErrorCheck(currentChannel->setPaused(false));									// Unpause sound
	}

	return nxtChannelId;
}

//--------------------------------------------------
// Set the position in 3D space
//--------------------------------------------------
void AudioEngine::SetChannel3DPosition(int channelId, const AudioVector3& position)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}

	FMOD_VECTOR newPosition = VectorToFmod(position);
	AudioEngine::ErrorCheck(foundIt->second->set3DAttributes(&newPosition, nullptr));
}

//--------------------------------------------------
// Move audio source position in 3D space
//--------------------------------------------------
void AudioEngine::MoveChannel3DPosition(int channelId, const AudioVector3& velocity)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}
	FMOD_VECTOR curPosition;
	foundIt->second->get3DAttributes(&curPosition, nullptr);
	curPosition.x += VectorToFmod(velocity).x;
	curPosition.y += VectorToFmod(velocity).y;
	curPosition.z += VectorToFmod(velocity).z;
	AudioEngine::ErrorCheck(foundIt->second->set3DAttributes(&curPosition, nullptr));
}

//--------------------------------------------------
// Set the volume of the sound channel
//--------------------------------------------------
void AudioEngine::SetChannelVolume(int channelId, float volume)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}

	AudioEngine::ErrorCheck(foundIt->second->setVolume(DecibalToVolume(volume)));
}

//--------------------------------------------------
// Return true if the channel is currently playing
//--------------------------------------------------
bool AudioEngine::IsPlaying(int channelId) const
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return false;
	}

	bool playing = false;
	AudioEngine::ErrorCheck(foundIt->second->isPlaying(&playing));
	return playing;
}

//--------------------------------------------------
// Stops all currently pl;aying channels
//--------------------------------------------------
void AudioEngine::StopAllChannels()
{
	bool playing;
	for (auto channelMap : Implementation::Instance()->audioChannels)
	{
		channelMap.second->isPlaying(&playing);
		if(playing)
			AudioEngine::ErrorCheck(channelMap.second->stop());
	}
	//Implementation::Instance()->nextChannelId = 0;
}

//--------------------------------------------------
// Stops the specified channel
//--------------------------------------------------
void AudioEngine::StopChannel(int channelId,float fadeOutTime)
{
	
	bool playing;
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}
	foundIt->second->isPlaying(&playing);
	if (playing)
	{
		unsigned long long dspClock;
		int rate;
		FMOD::System * sys;
		
		if(fadeOutTime <= 0.0f)
			AudioEngine::ErrorCheck(foundIt->second->stop());
		else
		{
			this->FadeOutChannel(foundIt->first, fadeOutTime, 0);
		}
	}
	//Implementation::Instance()->nextChannelId--;
}


//---------------------------------------------------------------
// Fade out a given channel to a given volume
// Stops the channel when fadeouttime is zero
// FadeOutTime is used to set the time over which the sound will fade
//---------------------------------------------------------------
void AudioEngine::FadeOutChannel(int channelId, float fadeOutTime, float fadeOutVolume)
{
	bool playing;
	float currVol;
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}
	AudioEngine::ErrorCheck(foundIt->second->isPlaying(&playing));
	AudioEngine::ErrorCheck(foundIt->second->getVolume(&currVol));
	if (playing)
	{
		unsigned long long dspClock;
		int rate;
		FMOD::System * sys;

		if (fadeOutTime <= 0.0f)
			AudioEngine::ErrorCheck(foundIt->second->stop());
		else
		{
			AudioEngine::ErrorCheck(foundIt->second->getSystemObject(&sys));
			AudioEngine::ErrorCheck(sys->getSoftwareFormat(&rate, 0, 0));
			AudioEngine::ErrorCheck(foundIt->second->getDSPClock(0, &dspClock));
			AudioEngine::ErrorCheck(foundIt->second->addFadePoint(dspClock, currVol));
			AudioEngine::ErrorCheck(foundIt->second->addFadePoint(dspClock + (rate * fadeOutTime), fadeOutVolume));
			AudioEngine::ErrorCheck(foundIt->second->setDelay(0, dspClock + (rate * fadeOutTime), true));
		}
	}
	foundIt->second->isPlaying(&playing);
	if (!playing)
	{
		Implementation::Instance()->nextChannelId--;
	}
}

//---------------------------------------------------------------
// Fade in a given channel to a given volume from zero volume
// FadeInTime is used to set the time over which the sound will fade
//---------------------------------------------------------------
void AudioEngine::FadeInChannel(int channelId, float fadeInTime, float fadeInVolume)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}
	
	unsigned long long dspClock;
	int rate;
	FMOD::System * sys;
	AudioEngine::ErrorCheck(foundIt->second->getSystemObject(&sys));
	AudioEngine::ErrorCheck(sys->getSoftwareFormat(&rate, 0, 0));
	AudioEngine::ErrorCheck(foundIt->second->getDSPClock(0, &dspClock));
	AudioEngine::ErrorCheck(foundIt->second->addFadePoint(dspClock, 0));
	AudioEngine::ErrorCheck(foundIt->second->addFadePoint(dspClock + (rate * fadeInTime), fadeInVolume));
}

//--------------------------------------------------
// Load bank files
//--------------------------------------------------
void  AudioEngine::LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags)
{
	auto foundIt = Implementation::Instance()->audioBanks.find(strBankName);
	if (foundIt != Implementation::Instance()->audioBanks.end())
		return;
	
	FMOD::Studio::Bank* currentBank;
	AudioEngine::ErrorCheck( Implementation::Instance()->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &currentBank));
	if (currentBank)
	{
		Implementation::Instance()->audioBanks[strBankName] = currentBank;
	}
}


//--------------------------------------------------
// Load FMOD studio events
//--------------------------------------------------
void AudioEngine::LoadEvents(const string& eventName)
{
	auto foundit = Implementation::Instance()->audioEvents.find(eventName);
	if (foundit != Implementation::Instance()->audioEvents.end())
		return;
	FMOD::Studio::EventDescription* eventDescription = NULL;
	AudioEngine::ErrorCheck(Implementation::Instance()->mpStudioSystem->getEvent(eventName.c_str(), &eventDescription));
	if (eventDescription)
	{
		FMOD::Studio::EventInstance* eventInstance = NULL;
		AudioEngine::ErrorCheck(eventDescription->createInstance(&eventInstance));
		if (eventInstance)
		{
			Implementation::Instance()->audioEvents[eventName] = eventInstance;
		}
	}
}

//--------------------------------------------------
// Play Events
//--------------------------------------------------
void AudioEngine::PlayEvents(const string &eventName)
{
	auto foundit = Implementation::Instance()->audioEvents.find(eventName);
	if (foundit == Implementation::Instance()->audioEvents.end())
	{
		LoadEvents(eventName);
		foundit = Implementation::Instance()->audioEvents.find(eventName);
		if (foundit == Implementation::Instance()->audioEvents.end())
			return;
	}
	foundit->second->start();
}

//--------------------------------------------------
// Stop Events
//--------------------------------------------------
void AudioEngine::StopEvent(const string &eventName, bool immediate)
{
	auto foundIt = Implementation::Instance()->audioEvents.find(eventName);
	if (foundIt == Implementation::Instance()->audioEvents.end())
		return;

	FMOD_STUDIO_STOP_MODE eMode;
	eMode = immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
	AudioEngine::ErrorCheck(foundIt->second->stop(eMode));
}

//--------------------------------------------------
// Check if the event is playing
//--------------------------------------------------
bool AudioEngine::IsEventPlaying(const string &eventName) const
{
	auto foundIt = Implementation::Instance()->audioEvents.find(eventName);
	if (foundIt == Implementation::Instance()->audioEvents.end())
		return false;

	FMOD_STUDIO_PLAYBACK_STATE* state = NULL;
	if (foundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING)
	{
		return true;
	}
	return false;
}

//--------------------------------------------------
// Get the parameter values of the events
//--------------------------------------------------
void AudioEngine::GetEventParameter(const string &eventName, const string &eventParameter, float* parameter)
{
	auto foundIt = Implementation::Instance()->audioEvents.find(eventName);
	if (foundIt == Implementation::Instance()->audioEvents.end())
		return;

	FMOD::Studio::ParameterInstance* parameterInstance = NULL;
	AudioEngine::ErrorCheck(foundIt->second->getParameter(eventParameter.c_str(), &parameterInstance));
	AudioEngine::ErrorCheck(parameterInstance->getValue(parameter));
}

//--------------------------------------------------
// Set the parameters of the events
//--------------------------------------------------
void AudioEngine::SetEventParameter(const string &eventName, const string &eventParameter, float value)
{
	auto foundIt = Implementation::Instance()->audioEvents.find(eventName);
	if (foundIt == Implementation::Instance()->audioEvents.end())
		return;

	FMOD::Studio::ParameterInstance* parameterInstance = NULL;
	AudioEngine::ErrorCheck(foundIt->second->getParameter(eventParameter.c_str(), &parameterInstance));
	AudioEngine::ErrorCheck(parameterInstance->setValue(value));
}

//--------------------------------------------------
// Set the pitch of a channel
// Pitch value, 0.5 = half pitch, 2.0 = double pitch, etc default = 1.0.
//--------------------------------------------------
void AudioEngine::SetPitch(int channelId, float pitch)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}

	AudioEngine::ErrorCheck(foundIt->second->setPitch(pitch));
}

//--------------------------------------------------
// Return the pitch of the given channel
//--------------------------------------------------
float AudioEngine::GetPitch(int channelId)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return 0;
	}
	float pitch = 0;
	AudioEngine::ErrorCheck(foundIt->second->getPitch(&pitch));
	return pitch;
}

//--------------------------------------------------
// Set the frequency of a channel
//--------------------------------------------------
void AudioEngine::SetFrequency(int channelId, float freq)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}

	AudioEngine::ErrorCheck(foundIt->second->setFrequency(freq));
}

//--------------------------------------------------
// Return the frequency of the given channel
// Frequency value in Hertz
// If frequency is negative the sound will play backwards
//--------------------------------------------------
float AudioEngine::GetFrequency(int channelId)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return 0;
	}
	float frequency = 0;
	AudioEngine::ErrorCheck(foundIt->second->getFrequency(&frequency));
	return frequency;
}

//--------------------------------------------------
// Set the low pass gain of a channel
//--------------------------------------------------
void AudioEngine::SetLowPassGain(int channelId, float gain)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return;
	}

	AudioEngine::ErrorCheck(foundIt->second->setLowPassGain(gain));
}

//--------------------------------------------------
// Return the low pass gain of the given channel
// Linear gain level, from 0 (silent, full filtering) to 1.0 (full volume, no filtering), default = 1.0.
//--------------------------------------------------
float AudioEngine::GetLowPassGain(int channelId)
{
	auto foundIt = Implementation::Instance()->audioChannels.find(channelId);
	if (foundIt == Implementation::Instance()->audioChannels.end())
	{
		return 0;
	}
	float gain = 0;
	AudioEngine::ErrorCheck(foundIt->second->getLowPassGain(&gain));
	return gain;
}

//-------------------------------------------------
// Activate environment reverb
//-------------------------------------------------
void AudioEngine::ActivateReverb(bool active)
{
	AudioEngine::ErrorCheck(Implementation::Instance()->reverb->setActive(active));
}

//-------------------------------------------------
// Return if reverb is active or not
//-------------------------------------------------
bool AudioEngine::GetReverbState() const
{
	bool active;
	AudioEngine::ErrorCheck(Implementation::Instance()->reverb->getActive(&active));
	return active;
}

//--------------------------------------------------
// Set a 3D reverb zone having the given reverb properties
// The zone will be a sphere centered at the gievn position
// The reverb effect will be heard from min distance from the center to the max distance
//--------------------------------------------------
void AudioEngine::SetEnvironmentReverb(const FMOD_REVERB_PROPERTIES reverbProperties, const AudioVector3& position, float minDist, float maxDist)
{
	AudioEngine::ErrorCheck(Implementation::Instance()->audioSystem->createReverb3D(&Implementation::Instance()->reverb));
	Implementation::Instance()->reverb->setProperties(&reverbProperties);
	FMOD_VECTOR pos = VectorToFmod(position);
	float mindist = minDist;
	float maxdist = maxDist;
	AudioEngine::ErrorCheck(Implementation::Instance()->reverb->set3DAttributes(&pos, mindist, maxdist));
}

//---------------------------------------------------------
// Set reverb properties
//---------------------------------------------------------
FMOD_REVERB_PROPERTIES AudioEngine::SetReverbProperties(const ReverbProperties& reverbProp) const
{
	FMOD_REVERB_PROPERTIES myReverbProp = { reverbProp.DecayTime, reverbProp.EarlyDelay, reverbProp.LateDelay, 
											reverbProp.HFReference, reverbProp.HFDecayRatio, reverbProp.Diffusion,
											reverbProp.Density, reverbProp.LowShelfFrequency, reverbProp.LowShelfGain, 
											reverbProp.HighCut, reverbProp.EarlyLateMix, reverbProp.WetLevel };
	return myReverbProp;
}

//--------------------------------------------------
// Convert from vector3 struct to FMOD vectors
//--------------------------------------------------
FMOD_VECTOR AudioEngine::VectorToFmod(const AudioVector3& position)
{
	FMOD_VECTOR fVec;
	fVec.x = position.x;
	fVec.y = position.y;
	fVec.z = position.z;
	return fVec;
}

//--------------------------------------------------
// Convert from decibal value to floating values
//--------------------------------------------------
float  AudioEngine::DecibalToVolume(float dB)
{
	return powf(10.0f, 0.05f * dB);
}

//--------------------------------------------------
// Convert from floating values to decibal value
//--------------------------------------------------
float  AudioEngine::VolumeToDecibal(float volume)
{
	return 20.0f * log10f(volume);
}

//--------------------------------------------------
// Cleanup FMOD resources
//--------------------------------------------------
void AudioEngine::ShutDown()
{
	delete Implementation::Instance();
}