#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
TapSynthAudioProcessorEditor::TapSynthAudioProcessorEditor (TapSynthAudioProcessor& p)
: AudioProcessorEditor (&p)
, audioProcessor (p)
, osc1 (audioProcessor.apvts, "OSC1", "OSC1GAIN", "OSC1PITCH","OSC1DETUNE", "DISTORTION1", "OSC1FMTYPE", "OSC1FMFREQ", "OSC1FMDEPTH", "OSC1NUMOBERTONES", "OSC1PAN", "OSC1UNISON")
, osc2 (audioProcessor.apvts, "OSC2", "OSC2GAIN", "OSC2PITCH","OSC2DETUNE", "DISTORTION2", "OSC2FMTYPE", "OSC2FMFREQ", "OSC2FMDEPTH", "OSC2NUMOBERTONES", "OSC2PAN", "OSC2UNISON")
, filter (audioProcessor.apvts, "FILTERTYPE", "FILTERCUTOFF", "FILTERRESONANCE")
, adsr (audioProcessor.apvts, "ATTACK", "DECAY", "SUSTAIN", "RELEASE")
, filterAdsr (audioProcessor.apvts, "FILTERATTACK", "FILTERDECAY", "FILTERSUSTAIN", "FILTERRELEASE")
, reverb (audioProcessor.apvts, "REVERBSIZE", "REVERBDAMPING", "REVERBWIDTH", "REVERBDRY", "REVERBWET", "REVERBFREEZE", "DELAY")
, meter (audioProcessor)
//==============================
, midiKeyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{   
    addAndMakeVisible (osc1);
    addAndMakeVisible (osc2);
    addAndMakeVisible (filter);
    addAndMakeVisible (adsr);
    addAndMakeVisible (filterAdsr);
    addAndMakeVisible (reverb);
    addAndMakeVisible (meter);
	addAndMakeVisible(midiKeyboard);
    
    osc1.setName ("Oscillator 1");
    osc2.setName ("Oscillator 2");
    filter.setName ("Filter");
    filterAdsr.setName ("Filter ADSR");
    adsr.setName ("ADSR");
    meter.setName ("Meter");
    
    auto oscColour = juce::Colour::fromRGB (247, 190, 67);
    auto filterColour = juce::Colour::fromRGB (246, 87, 64);
    
    osc1.setBoundsColour (oscColour);
    osc2.setBoundsColour (oscColour);
    
    filterAdsr.setBoundsColour (filterColour);
    filter.setBoundsColour (filterColour);
        
    startTimerHz (30);
    setSize (1030, 610);
	//midiKeyboard.grabKeyboardFocus();
}



TapSynthAudioProcessorEditor::~TapSynthAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TapSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void TapSynthAudioProcessorEditor::resized()
{
    const auto oscWidth = 390;
    const auto oscHeight = 190;
	//osc1.get
    osc1.setBounds (0, 0, oscWidth, oscHeight);
    osc2.setBounds (0, osc1.getBottom(), oscWidth, oscHeight);
    filter.setBounds (osc1.getRight(), 0, 180, 200);
    filterAdsr.setBounds (filter.getRight(), 0, 230, 360);
    adsr.setBounds (filterAdsr.getRight(), 0, 230, 360);
    reverb.setBounds (0, osc2.getBottom(), oscWidth+60, 150);
    meter.setBounds (reverb.getRight(), osc2.getBottom(), filterAdsr.getWidth()+ 200, 150);
    //logo.setBounds (meter.getRight(), osc2.getBottom() + 30, 250, 100);
	midiKeyboard.setBounds(8, meter.getBottom(), 1014, 70);
}

void TapSynthAudioProcessorEditor::timerCallback()
{
    repaint();
	//midiKeyboard.grabKeyboardFocus();
	//stopTimer();
}

void TapSynthAudioProcessorEditor::hostMIDIControllerIsAvailable(bool controllerIsAvailable)
{
	midiKeyboard.setVisible(!controllerIsAvailable);
}
