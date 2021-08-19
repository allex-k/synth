#include "OscComponent.h"

extern juce::StringArray oscTypes;
//==============================================================================
OscComponent::OscComponent(juce::AudioProcessorValueTreeState& apvts, 
	juce::String&& oscTypeId, juce::String&& gainId, juce::String&& pitchId, juce::String&& detuneId, juce::String&& distortionID, 
	juce::String&& fmTypeId, juce::String&& fmFreqId, juce::String&& fmDepthId, juce::String&& nObertonesId, 
	juce::String&& panId, juce::String&& unisonID)
: gain ("Gain", gainId, apvts, dialWidth, dialHeight)
, pitch ("Pitch", pitchId, apvts, dialWidth, dialHeight)
, detune("Detune", detuneId, apvts, dialWidth, dialHeight)
, distortion("Distortion", distortionID, apvts, dialWidth, dialHeight)
, fmFreq ("FM Freq", fmFreqId, apvts, dialWidth, dialHeight)
, fmDepth ("FM Depth", fmDepthId, apvts, dialWidth, dialHeight)
, nObertones("Num Obertones", nObertonesId, apvts, dialWidth+4, dialHeight+4)
, pan("Pan", panId, apvts, dialWidth + 4, dialHeight + 4)
, unison("Unison", unisonID, apvts, dialWidth, dialHeight)
{
    //juce::StringArray oscChoices { "Sine", "Saw", "Square", "Triangle", "Noice", "Saw smooth" };
    oscSelector.addItemList (oscTypes, 1);
    oscSelector.setSelectedItemIndex (0);
    addAndMakeVisible (oscSelector);

	fmTypeSelector.addItemList(oscTypes, 1);
	fmTypeSelector.setSelectedItemIndex(0);
	addAndMakeVisible(fmTypeSelector);
    
    oscSelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, oscTypeId, oscSelector);
	fmTypeSelectorlAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, fmTypeId, fmTypeSelector);

    addAndMakeVisible (gain);
    addAndMakeVisible (pitch);
	addAndMakeVisible(detune);
	addAndMakeVisible(distortion);
    addAndMakeVisible (fmFreq);
    addAndMakeVisible (fmDepth);

	addAndMakeVisible(nObertones);
	
	addAndMakeVisible(pan);
	addAndMakeVisible(unison);
	
}

OscComponent::~OscComponent()
{
}


void OscComponent::resized()
{
	const auto paddingX = 8;
	const auto paddingY = 8;
	const auto sliderWidth = dialWidth;
	const auto sliderHeight = sliderWidth +18;
	oscSelector.setBounds(paddingX, paddingY+25, 100, 25);
	gain.setBounds(oscSelector.getRight(), paddingY, sliderWidth, sliderHeight);
	pitch.setBounds(gain.getRight(), paddingY, sliderWidth, sliderHeight);
	detune.setBounds(pitch.getRight(), paddingY, sliderWidth, sliderHeight);
	distortion.setBounds(detune.getRight(), paddingY, sliderWidth, sliderHeight);

	fmTypeSelector.setBounds(paddingX, oscSelector.getBottom()+paddingY, 100, 25);
	pan.setBounds(paddingX, fmTypeSelector.getBottom(), sliderWidth, sliderHeight + 4);
	fmFreq.setBounds(fmTypeSelector.getRight(), gain.getBottom(), sliderWidth, sliderHeight);
	fmDepth.setBounds(gain.getRight(), gain.getBottom(), sliderWidth, sliderHeight);
	unison.setBounds(fmDepth.getRight(), gain.getBottom(), sliderWidth, sliderHeight);
	nObertones.setBounds(detune.getRight(), gain.getBottom(), sliderWidth, sliderHeight+4);
    
	/*auto yStart = 8;
    const auto width = 64;
    const auto height = 80;
    

    oscSelector.setBounds (18, 40, 100, 25);
    gain.setBounds (120, yStart, width, height);
    pitch.setBounds (120+ width, yStart, width, height);
	detune.setBounds(120 + width * 2, yStart, width, height);
	distortion.setBounds(120 + width * 3, yStart, width, height);

	yStart = gain.getBottom();
	fmTypeSelector.setBounds(18, yStart, 100, 25);
    fmFreq.setBounds (120, yStart, width, height);
    fmDepth.setBounds (120+ width, yStart, width, height);

	nObertones.setBounds(120 + width*3, yStart, width+8, height+8);*/
}
