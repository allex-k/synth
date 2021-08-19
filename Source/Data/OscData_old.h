//OscData.h

#pragma once

#include <JuceHeader.h>


class OscData : public juce::dsp::Oscillator<float>
{
public:
    void prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels);
	void setOscType(const int oscSelection);
	void setFmType(const int FmSelection);
    void setOscPitch (const int pitchOffset, const float detune);
    void setFreq (const int midiNoteNumber);
    void setFmOsc (const float freq, const float depth);
    //void renderNextBlock (juce::dsp::AudioBlock<float>& audioBlock);
    float processNextSample (float input);
	void setParams(const int oscChoice, const float oscGain, const int oscPitch, const float detune, const float distortion,
		const int FmChoice, const float fmFreq, const float fmDepth, const int nObertones);
    void resetAll();

private:
	float volume{ 0.3 };
	int midiNoteNumber{ 0 };
	int pitchOffset{ 0 };
	float detune{ 0.f };
	float distortion{ 1.f };
	
	juce::dsp::Oscillator<float> fmOsc;
	float fmDepth{ 0.0f };
	float freqOffsetForFM{ 0.0f };	
	juce::Random random;

	//void setType(juce::dsp::Oscillator<float>& osc, const int oscSelection);

	int oscType = -1;
	int fmType = -1;

	int nObertones = 0;
};

// return std::sin (x); //Sine Wave
// return x / MathConstants<float>::pi; // Saw Wave
// return x < 0.0f ? -1.0f : 1.0f;  // Square Wave
