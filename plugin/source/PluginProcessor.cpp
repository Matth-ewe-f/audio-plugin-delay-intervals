#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterFactory.h"

// === Constants ==============================================================
const float PluginProcessor::maxDelayTime = 250;
const int PluginProcessor::maxIntervals = 32;

// === Lifecycle ==============================================================
PluginProcessor::PluginProcessor()
	: AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
	tree(*this, nullptr, "PARAMETERS", createParameters()),
	lastSampleRate(44100),
	lastDelay(LONG_MAX)
{
#if PERFETTO
    MelatoninPerfetto::get().beginSession();
#endif
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].listenTo(&tree, getIdForLeftIntervalAmp(i));
		rightAmps[i].listenTo(&tree, getIdForRightIntervalAmp(i));
	}
}

PluginProcessor::~PluginProcessor()
{
#if PERFETTO
    MelatoninPerfetto::get().endSession();
#endif
}

juce::AudioProcessorValueTreeState::ParameterLayout
PluginProcessor::createParameters()
{
	juce::AudioProcessorValueTreeState::ParameterLayout parameters;
	parameters.add(ParameterFactory::createTimeParameter(
		"delay-time", "Delay Time", 20, maxDelayTime, 1, 100
	));
	parameters.add(ParameterFactory::createIntChoiceParameter(
		"num-intervals", "Intervals", juce::Array<int>(8, 16, maxIntervals), 1
	));
	// delay amplitudes
	for (int i = 0;i < maxIntervals;i++)
	{
		std::string leftName;
		std::string rightName;
		float lDefault;
		float rDefault;
		if (i == 0)
		{
			leftName = "Pre-Repeat Amplitude L";
			rightName = "Pre-Repeat Amplitude R";
			lDefault = 1;
			rDefault = 1;
		}
		else 
		{
			if (i < 10)
			{
				leftName = "Repeat 0" + std::to_string(i) + " Amplitude L";
				rightName = "Repeat 0" + std::to_string(i) + " Amplitude R";
			}
			else
			{
				leftName = "Repeat " + std::to_string(i) + " Amplitude L";
				rightName = "Repeat " + std::to_string(i) + " Amplitude R";
			}
			lDefault = 0;
			rDefault = 0;
		}
		parameters.add(ParameterFactory::createDelayAmpParameter(
			getIdForLeftIntervalAmp(i), leftName, lDefault
		));
		parameters.add(ParameterFactory::createDelayAmpParameter(
			getIdForRightIntervalAmp(i), rightName, rDefault
		));
	}
	return parameters;
}

// === Plugin Information =====================================================
bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

#if !JucePlugin_IsSynth
	// Check if the input matches the output
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif
	return true;
#endif
}

// === Process Audio ==========================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::ignoreUnused(samplesPerBlock);
	lastSampleRate = sampleRate;
	float maxSecondsDelay = (maxIntervals + 1) * (maxDelayTime / 1000);
	leftBuffer.resize(sampleRate, maxSecondsDelay);
	rightBuffer.resize(sampleRate, maxSecondsDelay);
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].smoothAmplitude.reset(samplesPerBlock);
		rightAmps[i].smoothAmplitude.reset(samplesPerBlock);
	}
}

void PluginProcessor::releaseResources() { }

void PluginProcessor::processBlock
(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
	juce::ignoreUnused(midiMessages);
	auto numInputChannels = getTotalNumInputChannels();
	auto numOutputChannels = getTotalNumOutputChannels();
	// zeroes out any unused outputs (if there are any)
	for (auto i = numInputChannels;i < numOutputChannels;i++)
		buffer.clear(i, 0, buffer.getNumSamples());
	// process each channel of the audio
	size_t numSamples = (size_t) buffer.getNumSamples();
	size_t delay = getDelaySamples();
	bool delayChanged = delay != lastDelay && lastDelay != LONG_MAX;
	for (int channel = 0;channel < numInputChannels;channel++)
	{
		float* channelData = buffer.getWritePointer(channel);
		CircularBuffer* circ = channel == 0 ? &leftBuffer : &rightBuffer;
		// add dry signal to delay buffer
		// if delay just stopped changing, fade the samples in (because the
		// delays would have been muted for a delay change)
		if (!lastBlockDelayChange)
		{
			circ->addSamples(channelData, numSamples);
		}
		else if (!delayChanged)
		{
			for (size_t i = 0;i < numSamples;i++)
			{
				float amp = (i + 1) / (float) numSamples;
				circ->addSample(channelData[i] * amp);
			}
		}
		// output dry signal
		for (size_t i = 0;i < numSamples;i++)
		{
			float amp;
			if (channel == 0)
				amp = getAmplitudeForLeftInterval(0);
			else
				amp = getAmplitudeForRightInterval(0);
			channelData[i] *= amp;
		}
		// if the delays have been faded out for a delay change, skip them
		if (delayChanged && lastBlockDelayChange)
			continue;
		// output each delay interval, scaled by its amplitude
		int curIntervals = getCurrentNumIntervals();
		for (int interval = 1;interval < curIntervals;interval++)
		{
			for (size_t j = 0;j < numSamples;j++)
			{
				float amp;
				if (channel == 0)
					amp = getAmplitudeForLeftInterval(interval);
				else
					amp = getAmplitudeForRightInterval(interval);
				size_t d = delayChanged ? lastDelay : delay;
				size_t s = (d * (size_t) interval) + (numSamples - j);
				float delayedSample = circ->getSampleDelayed(s);
				// if the delay value is changing, fade out the delays
				if (delayChanged)
					delayedSample *= (numSamples - j - 1) / (float) numSamples;
				// if the delay value stopped changing, fade in the delays
				// reaching this line implies delayChanged = false
				if (lastBlockDelayChange)
					delayedSample *= (j + 1) / (float) numSamples;
				channelData[j] += delayedSample * amp;
			}
		}
		// if the delay value has changed, clear the buffer
		if (delayChanged)
		{
			circ->clear();
		}
	}
	lastBlockDelayChange = delayChanged;
	lastDelay = delay;
}

// === Factory Functions ======================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
	return new PluginProcessor();
}

juce::AudioProcessorEditor *PluginProcessor::createEditor()
{
	return new PluginEditor(*this);
}

// === State ==================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{
	auto state = tree.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
	if (xml.get() != nullptr && xml->hasTagName(tree.state.getType()))
		tree.replaceState(juce::ValueTree::fromXml(*xml));
}


// === Private Helper =========================================================
size_t PluginProcessor::getDelaySamples()
{
	float ms = *tree.getRawParameterValue("delay-time");
	return (size_t) (lastSampleRate * ms / 1000);
}

int PluginProcessor::getCurrentNumIntervals()
{
	int value = (int) *tree.getRawParameterValue("num-intervals");
	if (value == 0)
		return 8;
	else if (value == 1)
		return 16;
	else if (value == 2)
		return 32;
	else
		return 0;
}

float PluginProcessor::getAmplitudeForLeftInterval(int index)
{
	return leftAmps[index].smoothAmplitude.getNextValue();
}

float PluginProcessor::getAmplitudeForRightInterval(int index)
{
	return rightAmps[index].smoothAmplitude.getNextValue();
}