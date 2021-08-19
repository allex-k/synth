//PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::StringArray oscTypes{ "Sine", "Saw", "Square", "Triangle", "Noice", "Saw smooth" };

//==============================================================================
TapSynthAudioProcessor::TapSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParams())
#endif
{
    synth.addSound (new SynthSound());     //[1]
    
    for (int i = 0; i < 5; i++)
    {
        synth.addVoice (new SynthVoice()); //[2]
    }

	delayBuffer.setSize(2, 12000);
	delayBuffer.clear();
}

TapSynthAudioProcessor::~TapSynthAudioProcessor()
{
    
}

//==============================================================================
const juce::String TapSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TapSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
	keyboardState.reset();
    
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    reverbParams.roomSize = 0.5f;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.5f;
    reverbParams.freezeMode = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.wetLevel = 0.0f;
    
    reverb.setParameters (reverbParams);

	//delay
}

void TapSynthAudioProcessor::releaseResources()
{
	keyboardState.reset();
	delayBuffer.clear();
	//audioBufferCopy.clear();
	// When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TapSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    setParams();

	keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true); //[1]
        
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples()); //[2]
    //juce::dsp::AudioBlock<float> block { buffer };
	applyDelay(buffer, delayBuffer, delayLevel);
	juce::dsp::AudioBlock<float> block{ buffer };
    reverb.process (juce::dsp::ProcessContextReplacing<float> (block));
    
	audioBufferCopy.makeCopyOf(buffer);
	
}

//==============================================================================
bool TapSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapSynthAudioProcessor::createEditor()
{
    return new TapSynthAudioProcessorEditor (*this);
}

//==============================================================================
void TapSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	if (auto xmlState = apvts.copyState().createXml())
		copyXmlToBinary(*xmlState, destData);
}

void TapSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
	if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
		apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapSynthAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout TapSynthAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // OSC select
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC1", "Oscillator 1", oscTypes, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC2", "Oscillator 2", oscTypes, 0));
    
    // OSC Gain
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC1GAIN", "Oscillator 1 Gain", juce::NormalisableRange<float> { -50.0f, 0.0f, 0.1f }, -10.0f, "dB"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC2GAIN", "Oscillator 2 Gain", juce::NormalisableRange<float> { -50.0f, 0.0f, 0.1f }, -10.0f, "dB"));
    
    // OSC Pitch val
    params.push_back (std::make_unique<juce::AudioParameterInt>("OSC1PITCH", "Oscillator 1 Pitch", -24, 24, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt>("OSC2PITCH", "Oscillator 2 Pitch", -24, 24, 0));
	//Detune
	params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1DETUNE", "Oscillator 1 Detune", juce::NormalisableRange<float> { -20.0f, 20.f, 0.1f }, 0.f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2DETUNE", "Oscillator 2 Detune", juce::NormalisableRange<float> { -20.0f, 20.f, 0.1f }, 0.f));

	//Distortion 
	params.push_back(std::make_unique<juce::AudioParameterFloat>("DISTORTION1", "Distortion1", juce::NormalisableRange<float> { 0.1f, 8.f, 0.02f, 0.6f}, 1.f, ""));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("DISTORTION2", "Distortion2", juce::NormalisableRange<float> { 0.1f, 8.f, 0.02f, 0.6f}, 1.f, ""));

	//=======================================================
	// FM type
	params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC1FMTYPE", "Oscillator 1 FM type", oscTypes, 0));
	params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC2FMTYPE", "Oscillator 2 FM type", oscTypes, 0));
    // FM Osc Freq
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC1FMFREQ", "Oscillator 1 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.1f,  0.3f}, 0.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC2FMFREQ", "Oscillator 2 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.1f,  0.3f}, 0.0f, "Hz"));
    
    // FM Osc Depth
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC1FMDEPTH", "Oscillator 1 FM Depth", juce::NormalisableRange<float> { 0.0f, 0.5f, 0.001f,  0.4f}, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("OSC2FMDEPTH", "Oscillator 2 FM Depth", juce::NormalisableRange<float> { 0.0f, 0.5f, 0.001f,  0.4f}, 0.0f, ""));
	
	params.push_back(std::make_unique<juce::AudioParameterInt>("OSC1NUMOBERTONES", "Oscillator 1 num obertones", 0, 8, 0));
	params.push_back(std::make_unique<juce::AudioParameterInt>("OSC2NUMOBERTONES", "Oscillator 2 num obertones", 0, 8, 0));
	
	//pan
	params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1PAN", "Oscillator 1 Pan", juce::NormalisableRange<float> { -1.f, 1.f, 0.01f }, 0.f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC2PAN", "Oscillator 2 Pan", juce::NormalisableRange<float> { -1.0f, 1.f, 0.01f }, 0.f));
	//unison
	params.push_back(std::make_unique<juce::AudioParameterInt>("OSC1UNISON", "Oscillator 1 unison", 1, 8, 1));
	params.push_back(std::make_unique<juce::AudioParameterInt>("OSC2UNISON", "Oscillator 2 uison", 1, 8, 1));
	//=======================================================

	 // ADSR
	params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", juce::NormalisableRange<float> { 0.f, 1.0f, 0.005f,  0.5f}, 0.01f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.005f, 0.5f}, 0.2f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", juce::NormalisableRange<float> { 0.f, 1.0f, 0.05f }, 0.05f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", juce::NormalisableRange<float> { 0.0f, 3.0f, 0.05f, 0.5f}, 1.f));


    //Filter
    params.push_back (std::make_unique<juce::AudioParameterChoice>("FILTERTYPE", "Filter Type", juce::StringArray { "Low Pass", "Band Pass", "High Pass" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("FILTERCUTOFF", "Filter Cutoff", juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.1f, 0.6f }, 20000.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("FILTERRESONANCE", "Filter Resonance", juce::NormalisableRange<float> { 0.1f, 2.0f, 0.1f }, 0.1f, ""));
    
	params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERADSRDEPTH", "Filter ADSR Depth", juce::NormalisableRange<float> { 0.0f, 10000.0f, 0.1f, 0.3f }, 10000.0f, ""));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERATTACK", "Filter Attack", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.01f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERDECAY", "Filter Decay", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.1f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERSUSTAIN", "Filter Sustain", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 1.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERRELEASE", "Filter Release", juce::NormalisableRange<float> { 0.0f, 3.0f, 0.1f }, 0.1f));

   
    // Reverb
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBSIZE", "Reverb Size", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBWIDTH", "Reverb Width", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 1.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBDAMPING", "Reverb Damping", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.5f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBDRY", "Reverb Dry", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 1.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBWET", "Reverb Wet", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>("REVERBFREEZE", "Reverb Freeze", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    
	params.push_back(std::make_unique<juce::AudioParameterFloat>("DELAY", "Delay", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.0f, ""));

    return { params.begin(), params.end() };
}

void TapSynthAudioProcessor::setParams()
{
    setVoiceParams();
    setFilterParams();
    setReverbParams();
}

void TapSynthAudioProcessor::setVoiceParams()
{
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            auto& attack = *apvts.getRawParameterValue ("ATTACK");
            auto& decay = *apvts.getRawParameterValue ("DECAY");
            auto& sustain = *apvts.getRawParameterValue ("SUSTAIN");
            auto& release = *apvts.getRawParameterValue ("RELEASE");
            
            auto& osc1Choice = *apvts.getRawParameterValue ("OSC1");
            auto& osc2Choice = *apvts.getRawParameterValue ("OSC2");
            auto& osc1Gain = *apvts.getRawParameterValue ("OSC1GAIN");
            auto& osc2Gain = *apvts.getRawParameterValue ("OSC2GAIN");
            auto& osc1Pitch = *apvts.getRawParameterValue ("OSC1PITCH");
            auto& osc2Pitch = *apvts.getRawParameterValue ("OSC2PITCH");
			auto& osc1Detune = *apvts.getRawParameterValue("OSC1DETUNE");
			auto& osc2Detune = *apvts.getRawParameterValue("OSC2DETUNE");
			auto& distortion1 = *apvts.getRawParameterValue("DISTORTION1");
			auto& distortion2 = *apvts.getRawParameterValue("DISTORTION2");
			auto& nObertones1 = *apvts.getRawParameterValue("OSC1NUMOBERTONES");
			auto& nObertones2 = *apvts.getRawParameterValue("OSC2NUMOBERTONES");
			auto& pan1 = *apvts.getRawParameterValue("OSC1PAN");
			auto& pan2 = *apvts.getRawParameterValue("OSC2PAN");
			auto& unison1 = *apvts.getRawParameterValue("OSC1UNISON");
			auto& unison2 = *apvts.getRawParameterValue("OSC2UNISON");

			auto& osc1FmType = *apvts.getRawParameterValue("OSC1FMTYPE");
			auto& osc2FmType = *apvts.getRawParameterValue("OSC2FMTYPE");
            auto& osc1FmFreq = *apvts.getRawParameterValue ("OSC1FMFREQ");
            auto& osc2FmFreq = *apvts.getRawParameterValue ("OSC2FMFREQ");
            auto& osc1FmDepth = *apvts.getRawParameterValue ("OSC1FMDEPTH");
            auto& osc2FmDepth = *apvts.getRawParameterValue ("OSC2FMDEPTH");	
            
            auto& filterAttack = *apvts.getRawParameterValue ("FILTERATTACK");
            auto& filterDecay = *apvts.getRawParameterValue ("FILTERDECAY");
            auto& filterSustain = *apvts.getRawParameterValue ("FILTERSUSTAIN");
            auto& filterRelease = *apvts.getRawParameterValue ("FILTERRELEASE");

            auto& osc1 = voice->getOscillator1();
            auto& osc2 = voice->getOscillator2();
            
            auto& adsr = voice->getAdsr();
            auto& filterAdsr = voice->getFilterAdsr();
            
			osc1.setParams({ osc1Choice, osc1Gain, osc1Pitch, osc1Detune,distortion1,osc1FmType, osc1FmFreq,osc1FmDepth,nObertones1, pan1, unison1});
			osc2.setParams({ osc2Choice, osc2Gain, osc2Pitch, osc2Detune, distortion2, osc2FmType, osc2FmFreq, osc2FmDepth, nObertones2, pan2, unison2});
			
            adsr.update (attack.load(), decay.load(), sustain.load(), release.load());
            filterAdsr.update (filterAttack, filterDecay, filterSustain, filterRelease);

			//===========
			
        }
    }
}

void TapSynthAudioProcessor::setFilterParams()
{
    auto& filterType = *apvts.getRawParameterValue ("FILTERTYPE");
    auto& filterCutoff = *apvts.getRawParameterValue ("FILTERCUTOFF");
    auto& filterResonance = *apvts.getRawParameterValue ("FILTERRESONANCE");
    auto& adsrDepth = *apvts.getRawParameterValue ("FILTERADSRDEPTH");
        
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->updateFilterParams (filterType, filterCutoff, filterResonance, adsrDepth);
        }
    }
}

void TapSynthAudioProcessor::setReverbParams()
{
    reverbParams.roomSize = *apvts.getRawParameterValue ("REVERBSIZE");
    reverbParams.width = *apvts.getRawParameterValue ("REVERBWIDTH");
    reverbParams.damping = *apvts.getRawParameterValue ("REVERBDAMPING");
    reverbParams.dryLevel = *apvts.getRawParameterValue ("REVERBDRY");
    reverbParams.wetLevel = *apvts.getRawParameterValue ("REVERBWET");
    reverbParams.freezeMode = *apvts.getRawParameterValue ("REVERBFREEZE");
    
    reverb.setParameters (reverbParams);

	delayLevel = *apvts.getRawParameterValue("DELAY");	
}

void TapSynthAudioProcessor::applyDelay(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, float delayLevel)
{
	auto numSamples = buffer.getNumSamples();

	auto delayPos = 0;

	for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
	{
		auto channelData = buffer.getWritePointer(channel);
		auto delayData = delayBuffer.getWritePointer(juce::jmin(channel, delayBuffer.getNumChannels() - 1));
		delayPos = delayPosition;

		for (auto i = 0; i < numSamples; ++i)
		{
			auto in = channelData[i];
			channelData[i] += delayData[delayPos];
			delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;

			if (++delayPos >= delayBuffer.getNumSamples())
				delayPos = 0;
		}
	}

	delayPosition = delayPos;
}