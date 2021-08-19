//SynthVoice.cpp
#include "SynthVoice.h"
bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    osc1.setFreq (midiNoteNumber);
    osc2.setFreq (midiNoteNumber);
    
    adsr.noteOn();
    filterAdsr.noteOn();
}

void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();
    filterAdsr.noteOff();
    
    if (! allowTailOff || ! adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    
}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    reset();
    
    adsr.setSampleRate (sampleRate);
    filterAdsr.setSampleRate (sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    osc1.prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
    osc2.prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
    filter.prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
    
    isPrepared = true;
}

void SynthVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    if (! isVoiceActive())
        return;
    
    synthBuffer.setSize (outputBuffer.getNumChannels(), numSamples, false, false, true);
	synthBuffer.clear();

	filterAdsr.applyEnvelopeToBuffer(synthBuffer, 0, numSamples); //!TO DO filter adsr afer oscilator 
	filterAdsrOutput = filterAdsr.getNextSample(); 

	osc1.renderNextBlock(synthBuffer);
	osc2.renderNextBlock(synthBuffer);
    
	filter.processNextBlock(synthBuffer);

    adsr.applyEnvelopeToBuffer (synthBuffer, 0, numSamples);
         
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom (channel, startSample, synthBuffer, channel, 0, numSamples);
        
        if (! adsr.isActive())
            clearCurrentNote();
    }
}

void SynthVoice::reset()
{
    adsr.reset();
    filterAdsr.reset();
}

void SynthVoice::updateFilterParams (const int filterType, const float filterCutoff, const float filterResonance, const float adsrDepth/*const float lfoFreq, const float lfoDepth*/)
{
    auto cutoff = (adsrDepth * filterAdsrOutput) + filterCutoff;
    cutoff = std::clamp<float> (cutoff, 20.0f, 20000.0f);

        filter.setParams (filterType, cutoff, filterResonance);
}
