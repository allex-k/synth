//SynthVoice.h
#pragma once

#include "SynthSound.h"
#include "Data/OscData.h"
#include "Data/FilterData.h"
#include "Data/AdsrData.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples) override;
    
    void reset();
    
	OscData& getOscillator1() { return osc1; }
	OscData& getOscillator2() { return osc2; }
    AdsrData& getAdsr() { return adsr; }
    AdsrData& getFilterAdsr() { return filterAdsr; }
    float getFilterAdsrOutput() { return filterAdsrOutput; }
    void updateFilterParams(const int filterType, const float filterCutoff, const float filterResonance, const float adsrDepth /*const float lfoFreq, const float lfoDepth*/);
    
private:
    static constexpr int numChannelsToProcess { 2 };
    OscData osc1;
    OscData osc2;
    FilterData filter;
    AdsrData adsr;
    AdsrData filterAdsr;
    juce::AudioBuffer<float> synthBuffer;
    float filterAdsrOutput { 0.0f };
    bool isPrepared { false };
};
