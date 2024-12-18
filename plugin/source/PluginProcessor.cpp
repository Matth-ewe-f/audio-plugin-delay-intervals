#include "PluginProcessor.h"
#include <juce_dsp/juce_dsp.h>
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
	lastSampleRate(44100)
{
#if PERFETTO
    MelatoninPerfetto::get().beginSession();
#endif
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].listenTo(&tree, getIdForLeftIntervalAmp(i));
		rightAmps[i].listenTo(&tree, getIdForRightIntervalAmp(i));
	}
	lastDelay = getDelaySamples();
	lastBlockDelayChanged = false;
	lastDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	lastFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	lastLoop = *tree.getRawParameterValue("loop") >= 1;
	for (size_t i = 0;i < maxIntervals;i++)
	{
		Filter* left = &leftDelays[i].filter;
		left->attachToParameters(&tree, "left-high-pass", "left-low-pass");
		Filter* right = &rightDelays[i].filter;
		right->attachToParameters(&tree, "right-high-pass", "right-low-pass");
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
	parameters.add(ParameterFactory::createBoolParameter(
		"loop", "Loop", "ON", "OFF", 0
	));
	parameters.add(ParameterFactory::createPercentageParameter(
		"dry-wet", "Dry/Wet", 50
	));
	parameters.add(ParameterFactory::createPercentageParameter(
		"falloff", "Auto-Falloff", 0
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"left-high-pass", "Filter Left Low", 20
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"left-low-pass", "Filter Left High", 20000
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"right-high-pass", "Filter Right Low", 20
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"right-low-pass", "Filter Right High", 20000
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
	lastSampleRate = sampleRate;
	// spec for filters
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = (unsigned) samplesPerBlock;
	spec.numChannels = (unsigned) getTotalNumOutputChannels();
	// prepare delay intervals
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].reset(samplesPerBlock);
		rightAmps[i].reset(samplesPerBlock);
		float seconds = (maxDelayTime / 1000) + 0.01f;
		leftDelays[i].buffer.resize(sampleRate, seconds);
		leftDelays[i].filter.prepare(spec);
		rightDelays[i].buffer.resize(sampleRate, seconds);
		rightDelays[i].filter.prepare(spec);
	}
	// prepare trackign of smoothed values
	lastDelay = getDelaySamples();
	lastBlockDelayChanged = false;
	lastDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	lastFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	lastLoop = *tree.getRawParameterValue("loop") >= 1;
}

void PluginProcessor::releaseResources() { }

void PluginProcessor::processBlock
(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
	TRACE_DSP();
	juce::ignoreUnused(midiMessages); // not a midi plugin
	auto numInputChannels = getTotalNumInputChannels();
	auto numOutputChannels = getTotalNumOutputChannels();
	// zeroes out any unused outputs (if there are any)
	for (auto i = numInputChannels;i < numOutputChannels;i++)
		buffer.clear(i, 0, buffer.getNumSamples());
	// get parameters by which to process the audio
	size_t numSamples = (size_t) buffer.getNumSamples();
	size_t curDelay = getDelaySamples();
	bool delayChanged = curDelay != lastDelay && lastDelay != LONG_MAX;
	size_t delay = delayChanged ? lastDelay : curDelay;
	float curDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	float curFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	bool loop = *tree.getRawParameterValue("loop") >= 1;
	// process each channel
	for (int channel = 0;channel < numInputChannels;channel++)
	{
		// get parameters by which to process the channel
		float* channelData = buffer.getWritePointer(channel);
		DelayAmp* amplitudes = channel == 0 ? leftAmps : rightAmps;
		DelayInterval* intervals = channel == 0 ? leftDelays : rightDelays;
		// Filter* filter = channel == 0 ? &leftFilter : &rightFilter;
		float dryWet = lastDryWet;
		float dryWetSetp = (curDryWet - lastDryWet) / numSamples;
		float falloff = lastFalloff;
		float falloffStep = (curFalloff - lastFalloff) / numSamples;
		size_t curIntervals = getCurrentNumIntervals();
		// process each sample
		for (size_t i = 0;i < numSamples;i++)
		{
			float drySample = channelData[i];
			// add the dry signal to the output
			dryWet += dryWetSetp;
			channelData[i] *= (1 - dryWet) * amplitudes[0].getAmplitude();
			// if the delay value is changing, skip processing the delays
			if (delayChanged && lastBlockDelayChanged)
				continue;
			// smooth values by by which to adjust the samples
			falloff += falloffStep;
			float fade = 1;
			if (delayChanged)
				fade = (numSamples - i - 1) / (float) numSamples;
			else if (lastBlockDelayChanged)
				fade = (i + 1) / (float) numSamples;
			// add each delay intervals next sample to the output, in reverse
			// order so each interval adds its samples to the output before
			// recieving new samples
			for (size_t j = curIntervals - 1;j <= 1000;j--)
			{
				// add the sample to the output
				DelayInterval* interval = intervals + j;
				float sample = interval->buffer.getSampleDelayed(delay);
				float amp = amplitudes[j].getAmplitude();
				float processedSample = sample * dryWet * falloff * amp;
				if (delayChanged)
					processedSample *= fade;
				channelData[i] += processedSample;
				// add the sample to the subsequent delay interval's buffer
				if (j == 0)
				{
					DelayInterval* next = intervals + 1;
					// TODO apply first filter to drySample here
					// TODO apply second filter to (sample * falloff) here
					float sampleForNext = (sample * falloff) + drySample;
					sampleForNext = next->filter.processSample(sampleForNext);
					if (lastBlockDelayChanged) // implies !delayChanged
						sampleForNext *= fade;
					next->buffer.addSample(sampleForNext);
				}
				else if (j != curIntervals - 1)
				{
					DelayInterval* next = (intervals + j + 1);
					float sampleForNext = sample * falloff;
					// TODO apply second filter to sampleForNext here
					sampleForNext = next->filter.processSample(sampleForNext);
					if (lastBlockDelayChanged) // implies !delayChanged
						sampleForNext *= fade;
					next->buffer.addSample(sampleForNext);
				}
			}
			// loop samples from the last delay interval to the first
			if (loop || lastLoop)
			{
				size_t last = curIntervals - 1;
				float sample = intervals[last].buffer.getSampleDelayed(delay);
				float loopFade;
				if (loop && lastLoop)
					loopFade = 1;
				else if (!lastLoop)
					loopFade = (i + 1) / (float) numSamples;
				else
					loopFade = (numSamples - i - 1) / (float) numSamples;
				intervals[0].buffer.addSample(sample * falloff * loopFade);
			}
			else
			{
				intervals[0].buffer.addSample(0);
			}
		}
		// if the delay value is changing, zero out delay interval buffers
		if (delayChanged && !lastBlockDelayChanged)
		{
			for (size_t i = 0;i < maxIntervals;i++)
			{
				intervals[i].buffer.clear();
			}
		}
	}
	// save parameters to compare with next block
	lastDelay = curDelay;
	lastBlockDelayChanged = delayChanged;
	lastDryWet = curDryWet;
	lastFalloff = curFalloff;
	lastLoop = loop;
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

size_t PluginProcessor::getCurrentNumIntervals()
{
	int value = (int) *tree.getRawParameterValue("num-intervals");
	if (value == 0)
		return 8UL;
	else if (value == 1)
		return 16UL;
	else if (value == 2)
		return 32UL;
	else
		return 0UL;
}