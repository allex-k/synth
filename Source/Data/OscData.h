//OscData.h
#pragma once
#include <JuceHeader.h>
const unsigned int UNISON_MAX = 8;
typedef struct
{
	float oscChoice = 0, oscGain = -10, oscPitch = 0, detune = 0, distortion = 1,
		FmChoice = 0, fmFreq = 0, fmDepth = 0, nObertones = 0,
		pan = 0, unison = 1;

} OscParams;


class OscData
{
public:
	void renderNextBlock(juce::AudioBuffer<float>& synthBuffer);

	void setFreq(const int midiNoteNumber);

	void prepareToPlay(double sampleRate, int /*samplesPerBlock*/, int /*outputChannels*/);
	
	void setParams(const OscParams& params);
	void resetAll();

private:
	float a1{ 0.3 };
	float f1{ 0.f };
	int midiNoteNumber{ 0 };
	int pitchOffset{ 0 };
	float detune{ 0.f };
	float distortion{ 1.f };

	float f2{ 0.0f };
	float a2{ 0.0f };
	juce::dsp::Phase<float> O1Phases[UNISON_MAX];
	//juce::dsp::Phase<float> O1Phase;
	juce::dsp::Phase<float> O2Phase;
	std::function<float(float)> Osc1;
	std::function<float(float)> Osc2;

	int oscType = -1;
	int fmType = -1;

	int nObertones = 0;
	float pan = 0;
	int unison = 1;

	float sampleRate{ 48000.f };
	
	bool lookupTablesActual = false;
	std::unique_ptr<juce::dsp::LookupTableTransform<float>> lookupTableO1, lookupTableO2;

	void createLookupTableO1(const int osc1Selection);
	void createLookupTableO2(const int osc2Selection);

};