//OscData.cpp

#include "OscData.h"
void OscData::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    resetAll();
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    prepare (spec);
    fmOsc.prepare (spec);
    //gain.prepare (spec);    
}
#define PI  juce::MathConstants<float>::pi

float triangle(float x) { return std::asin(std::sin(x))*2.f / PI; }
float saw(float x) { return std::atan(std::tan(-x/2.f + PI/2))*2.f / PI; }
float square(float x) { return std::floor(std::cos(x))*2.f + 1.f; }
float sawSmooth(float x) {
	float output = 0.f, dCustom = 10.f;

	for (float n = 1.f; n < dCustom; n++)
		output += (std::sin(n*x)) / n;
	return output * 2.f / PI;
}

void OscData::setOscType(const int oscSelection)
{
	const size_t numPoints = 128;
	float (*funcPtr)(float) = std::sin;

    switch (oscSelection)
    {
        // Sine
        case 0: funcPtr = std::sin; break;
        // Saw
        case 1: funcPtr = saw; break;
        // Square
        case 2: funcPtr = square; break;
		// Triangle
		case 3: funcPtr = triangle; break;	
		case 4: initialise([&](float x) { return random.nextFloat()*2.f - 1.f; }); break;
		case 5: funcPtr = sawSmooth;  break;
        default: jassertfalse; break;
    }
	int nObertonesCopy = nObertones;
	if (oscSelection != 4) 
	{ 
		initialise([nObertonesCopy, funcPtr](float x)
		{
			float sum = 0.f;
			for (int i = 1; i <= 1+ nObertonesCopy; ++i) {
				sum += funcPtr(i*x);
			}
			sum /= (1.f + nObertonesCopy);
			return sum;
		}, numPoints);
	}
}

void OscData::setFmType(const int FmSelection) 
{
	switch (FmSelection)
	{
		// Sine
	case 0:
		fmOsc.initialise([](float x) { return std::sin(x); }); break;
		// Saw
	case 1:
		fmOsc.initialise([](float x) { return -x / PI; }); break;
		// Square
	case 2:
		fmOsc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; }); break;
		// Triangle
	case 3:
		fmOsc.initialise([](float x) { return std::asin(std::sin(x))*2.f / PI; }); break;
	case 4:
		fmOsc.initialise([&](float x) { return random.nextFloat()*2.f - 1.f; }); break;
	case 5:
		fmOsc.initialise([](float x) {return sawSmooth(x); }); break;
	default: jassertfalse; break;
	}
}

void OscData::setFreq (const int midiNoteNumber){this->midiNoteNumber = midiNoteNumber;}
void OscData::setOscPitch(const int pitchOffset, const float detune) { this->pitchOffset = pitchOffset; this->detune = detune; }

void OscData::setFmOsc (const float freq, const float depth)
{
    fmOsc.setFrequency (freq);
	fmDepth = depth;
}

/*void OscData::renderNextBlock (juce::dsp::AudioBlock<float>& audioBlock)
{
    jassert (audioBlock.getNumSamples() > 0);
    process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
}
*/
float OscData::processNextSample (float input)
{
	float mainFreq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber + pitchOffset)+detune;
	
    freqOffsetForFM = mainFreq * fmDepth * fmOsc.processSample(0.f);
	setFrequency(mainFreq + freqOffsetForFM, true);

	float output = processSample(0.f);
	output = (output > 0.f) ? pow(output, distortion) : -pow(abs(output), distortion); //distortion;

    return volume * output;
}

void OscData::setParams(const int oscChoice, const float oscGain, const int oscPitch, const float detune, const float distortion,
	const int FmChoice, const float fmFreq, const float fmDepth, const int nObertones)
{
	setOscType(oscChoice);
	setFmType(FmChoice);
    setOscPitch (oscPitch, detune);
    setFmOsc (fmFreq, fmDepth);
	this->volume= std::pow(10.f, oscGain / 20.f);
	this->distortion = distortion;
	this->nObertones = nObertones;
}

void OscData::resetAll()
{
    reset();
    fmOsc.reset();
	volume = 0.3f;
	distortion = 1.f;
}
