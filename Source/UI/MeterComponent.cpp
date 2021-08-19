
#include "MeterComponent.h"

//==============================================================================
MeterComponent::MeterComponent (TapSynthAudioProcessor& p) : audioProcessor (p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

MeterComponent::~MeterComponent()
{
}

inline float scale(float v, float a, float b, float x, float y) {
	return (v - a) / (b - a)*(y - x) + x;
}

void MeterComponent::paintOverChildren(juce::Graphics& g)
{
	  //===============================================
	
	auto buffer = audioProcessor.getAudioBufferCopy();
	auto samplesPtr = buffer.getReadPointer(0);
	
	if (samplesPtr != nullptr) {
		auto numSamples = buffer.getNumSamples();

		float paddingX = 8;

		g.setColour(juce::Colours::aqua);
		float width = getRight() - paddingX * 2, height = getHeight();
		float xStep = width / numSamples;

		float x = paddingX, y = scale(samplesPtr[0], -3.f, 3.f, height, 0.f);
		float xNew, yNew;
		for (int i = 1; i < numSamples; ++i)
		{
			xNew = x + xStep;
			yNew = scale(samplesPtr[i], -3.f, 3.f, height, 0.f);
			g.drawLine(x, y, xNew, yNew);
			x = xNew; y = yNew;
		}
		//========================
		float rms = buffer.getRMSLevel(0, 0, numSamples);
		float magnitude = buffer.getMagnitude(0, 0, numSamples);

		g.fillRoundedRectangle(paddingX, height - height / 8, magnitude * 100, height / 8 - paddingX, 2);
		g.setColour(juce::Colours::mediumaquamarine);
		g.fillRoundedRectangle(paddingX, height - height / 8, rms * 100, height / 8 - paddingX, 2);
		//buffer.max
	}

}
	

void MeterComponent::resized()
{
   
}
