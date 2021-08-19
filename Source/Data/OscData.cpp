//OscData.cpp
#include "OscData.h"
#define PI  juce::MathConstants<float>::pi
#define TWOPI  juce::MathConstants<float>::twoPi

float clip(float val, float min=0.f, float max=1.f) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}
void gen_unison_frequencies(float* frequencies, float f1, int unison, float detune) {
	if (unison == 1) {frequencies[0] = f1; return;}
	float maxDetune = f1 * detune;
	float step = maxDetune / unison * 2;
	float currentFreq = f1 - maxDetune;
	for (int i = 0; i < unison; ++i) {
		frequencies[i] = currentFreq;
		currentFreq += step;
	}
}
void OscData::renderNextBlock(juce::AudioBuffer<float>& synthBuffer) {
	int numSamples = synthBuffer.getNumSamples();
	float *bufferL, *bufferR;
	bufferL = synthBuffer.getWritePointer(0, 0);
	bufferR = synthBuffer.getWritePointer(1, 0);
	
	float leftCoeff = pow(clip(1 - pan), 2.f),
		rightCoeff = pow(clip(pan + 1), 2.f);

	//synth oscillator with LFO FM 
	//sample = a1 * O1(TWOPI*f1*s + a2 * f1*O2(TWOPI*f2*p));
	//float samples[UNISON_MAX];
	
	float frequencies[UNISON_MAX];

	f1 = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber + pitchOffset) + detune;
	gen_unison_frequencies(frequencies, f1, unison, 0.01);

	for (int s = 0; s < numSamples; ++s)
	{
		float O2PhaseIncrement = TWOPI * f2 / sampleRate;
		float O2PhaseVal = O2Phase.advance(O2PhaseIncrement) - PI;

		float sample = 0;
		for (int i = 0; i < unison; ++i) {
			float O1PhaseIncrement = TWOPI * frequencies[i] / sampleRate + a2 * frequencies[i] * Osc2(O2PhaseVal);
			float O1PhaseVal = O1Phases[i].advance(O1PhaseIncrement) - PI;
			sample += a1 * Osc1(O1PhaseVal)/ unison;
		}

		sample = (sample > 0.f) ? pow(sample, distortion) : -pow(abs(sample), distortion);

		//stereo panoram
		bufferL[s] += leftCoeff * sample; bufferR[s] += rightCoeff * sample;
	}
}

/*void OscData::renderNextBlock(juce::AudioBuffer<float>& synthBuffer) {
	int numSamples = synthBuffer.getNumSamples();
	float *bufferL, *bufferR;
	bufferL = synthBuffer.getWritePointer(0, 0);
	bufferR = synthBuffer.getWritePointer(1, 0);
	
	//synth oscillator with LFO FM 
	//sample = a1 * O1(TWOPI*f1*s + a2 * f1*O2(TWOPI*f2*p));
	float samples;
	f1 = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber + pitchOffset) + detune;
	float leftCoeff = pow(clip(1 - pan), 2.f), 
		rightCoeff = pow(clip(pan + 1), 2.f);
	
	for (int s = 0; s < numSamples; ++s)
	{
		float O2PhaseIncrement = TWOPI * f2 / sampleRate;
		float O2PhaseVal = O2Phase.advance(O2PhaseIncrement) - PI;

		float O1PhaseIncrement = TWOPI * f1 / sampleRate + a2 * f1*Osc2(O2PhaseVal);
		float O1PhaseVal = O1Phase.advance(O1PhaseIncrement) -PI;
		float sample = a1 * Osc1(O1PhaseVal);
		
		
		sample = (sample > 0.f) ? pow(sample, distortion) : -pow(abs(sample), distortion);
		
		//stereo panoram
		bufferL[s] += leftCoeff* sample; bufferR[s] += rightCoeff*sample;
	}
}*/

void OscData::prepareToPlay(double sampleRate, int /*samplesPerBlock*/, int /*outputChannels*/) { this->sampleRate = sampleRate; }

void OscData::setFreq(const int midiNoteNumber) { this->midiNoteNumber = midiNoteNumber; }

void OscData::setParams(const OscParams& params)
{
	a1 = std::pow(10.f, params.oscGain / 20.f);
	pitchOffset = params.oscPitch;
	detune = params.detune;
	distortion = params.distortion;
	f2 = params.fmFreq;
	a2 = params.fmDepth/2000.f;
	pan = params.pan;
	unison = params.unison;

	if (oscType != (int)params.oscChoice || nObertones != (int)params.nObertones) { oscType = params.oscChoice; nObertones = params.nObertones; createLookupTableO1(oscType); }
	if (fmType != (int)params.FmChoice) { fmType = params.FmChoice; createLookupTableO2(fmType); }
}

float triangle(float x) { return std::asin(std::sin(x))*2.f / PI; }
float saw(float x) { return std::atan(std::tan(-x / 2.f + PI / 2))*2.f / PI; }
float sawSmooth(float x) {
	float output = 0.f, dCustom = 12.f;

	for (float n = 1.f; n < dCustom; n++)
		output += (std::sin(n*x)) / n;
	return output * 2.f / PI;
}
float square(float x) { return std::floor(std::sin(x))*2.f + 1.f; }
float randomFloat(float x) { return rand() / float(RAND_MAX) * 2.f - 1.f; }


void OscData::createLookupTableO1(const int osc1Selection)
{
	const size_t lookupTableNumPoints = 128;
	
	float(*funcPtr)(float) = std::sin;
	switch (osc1Selection)
	{
	case 1: funcPtr = saw; break;
	case 2: funcPtr = square; break;
	case 3: funcPtr = triangle; break;
	//case 4: funcPtr = randomFloat; break;
	case 5: funcPtr = sawSmooth;  break;
	default: jassertfalse; break;
	}
	int nObertonesCopy = nObertones;

	auto* table = new juce::dsp::LookupTableTransform<float>(
	[nObertonesCopy, funcPtr](float x) {
		float sum = 0.f;
		for (int i = 1; i <= 1 + nObertonesCopy; ++i)
			sum += funcPtr(i*x);
		sum /= (1.f + nObertonesCopy);
		return sum;
	}
		,-PI, PI, lookupTableNumPoints);

	lookupTableO1.reset(table);
	Osc1 = [table](float x) { return (*table) (x); };
	if (osc1Selection == 4) Osc1 = randomFloat;
}

void OscData::createLookupTableO2(const int osc2Selection)
{
	const size_t lookupTableNumPoints = 128;

	float(*funcPtr)(float) = std::sin;
	switch (osc2Selection)
	{
	case 1: funcPtr = saw; break;
	case 2: funcPtr = square; break;
	case 3: funcPtr = triangle; break;
	//case 4: funcPtr = randomFloat; break;
	case 5: funcPtr = sawSmooth;  break;
	default: jassertfalse; break;
	}

	auto* table = new juce::dsp::LookupTableTransform<float>(funcPtr, -PI, PI, lookupTableNumPoints);

	lookupTableO2.reset(table);
	Osc2 = [table](float x) { return (*table) (x); };
	if (osc2Selection == 4) Osc2 = randomFloat;
}

