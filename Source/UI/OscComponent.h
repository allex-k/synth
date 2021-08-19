#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"

//==============================================================================
/*
*/
class OscComponent : public CustomComponent
{
public:
	OscComponent(juce::AudioProcessorValueTreeState& apvts,
		juce::String&& oscTypeId, juce::String&& gainId, juce::String&& pitchId, juce::String&& detuneId, juce::String&& distortionID, 
		juce::String&& fmTypeId, juce::String&& fmFreqId, juce::String&& fmDepthId, juce::String&& nObertonesId,
		juce::String&& panId, juce::String&& unisonID);
    ~OscComponent() override;

    void resized() override;

private:
    juce::ComboBox oscSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelAttachment;
	juce::ComboBox fmTypeSelector;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> fmTypeSelectorlAttachment;

    SliderWithLabel gain;
    SliderWithLabel pitch;
	SliderWithLabel detune;
	SliderWithLabel distortion;
    SliderWithLabel fmFreq;
    SliderWithLabel fmDepth;
	SliderWithLabel nObertones;
	SliderWithLabel pan;
	SliderWithLabel unison;
	
    
    static constexpr int dialWidth = 64;
    static constexpr int dialHeight = 64;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscComponent)
};
