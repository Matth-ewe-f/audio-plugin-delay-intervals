#include "PluginProcessor.h"
#include <algorithm>
#include <juce_dsp/juce_dsp.h>
#include "PluginEditor.h"
#include "ParameterFactory.h"

// === Constants ==============================================================
const float PluginProcessor::maxDelayTime = 250;

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
	std::string lHigh = "left-high-pass";
	std::string lLow = "left-low-pass";
	std::string lMix = "left-filter-mix";
	std::string rHigh = "right-high-pass";
	std::string rLow = "right-low-pass";
	std::string rMix = "right-filter-mix";
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].listenTo(&tree, getIdForLeftIntervalAmp(i));
		rightAmps[i].listenTo(&tree, getIdForRightIntervalAmp(i));
		leftFilters[i].attachToParameters(&tree, lHigh, lLow, lMix);
		rightFilters[i].attachToParameters(&tree, rHigh, rLow, rMix);
	}
	lastDelay = getDelaySamples();
	lastIntervals = getCurrentNumIntervals();
	lastBlockFadeOut = false;
	lastDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	lastFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	lastLoop = *tree.getRawParameterValue("loop") >= 1;
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
	juce::StringArray options = juce::StringArray(
		"16th triplet",
		"16th",
		"16th dotted",
		"8th triplet",
		"8th",
		"8th dotted",
		"4th triplet",
		"4th",
		"4th dotted"
	);
	parameters.add(ParameterFactory::createChoiceParameter(
		"delay-time-sync", "Delay Time (Rhythmic)", options, 4
	));
	parameters.add(ParameterFactory::createBoolParameter(
		"tempo-sync", "Tempo Sync", "ON", "OFF", 0
	));
	parameters.add(ParameterFactory::createIntParameter(
		"num-intervals", "Intervals", 8, 16, 8
	));
	parameters.add(ParameterFactory::createBoolParameter(
		"loop", "Loop", "ON", "OFF", 0
	));
	parameters.add(ParameterFactory::createBoolParameter(
		"delays-linked", "Delays Mirrored", "ON", "OFF", 0
	));
	parameters.add(ParameterFactory::createBoolParameter(
		"filters-linked", "Filters Mirrored", "ON", "OFF", 0
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
	parameters.add(ParameterFactory::createPercentageParameter(
		"left-filter-mix", "Filter Left Mix", 100
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"right-high-pass", "Filter Right Low", 20
	));
	parameters.add(ParameterFactory::createFreqParameter(
		"right-low-pass", "Filter Right High", 20000
	));
	parameters.add(ParameterFactory::createPercentageParameter(
		"right-filter-mix", "Filter Right Mix", 100
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
	// prepare the filters
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = (unsigned) samplesPerBlock;
	spec.numChannels = (unsigned) getTotalNumOutputChannels();
	for (int i = 0;i < maxIntervals;i++)
	{
		leftFilters[i].prepare(spec);
		rightFilters[i].prepare(spec);
	}
	// prepare the delay buffers
	leftBuffer.resize(sampleRate, (maxDelayTime / 1000) * (maxIntervals + 1));
	rightBuffer.resize(sampleRate, (maxDelayTime / 1000) * (maxIntervals + 1));
	// prepare tracking of smoothed values
	lastDelay = getDelaySamples();
	lastIntervals = getCurrentNumIntervals();
	lastBlockFadeOut = false;
	lastDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	lastFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	lastLoop = *tree.getRawParameterValue("loop") >= 1;
	tempBuffer.resize((size_t) samplesPerBlock, 0.0f);
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
	size_t curIntervals = getCurrentNumIntervals();
	bool intervalsChanged = curIntervals != lastIntervals;
	size_t intervals = intervalsChanged ? lastIntervals : curIntervals;
	bool fadeOut = delayChanged || intervalsChanged;
	float curDryWet = *tree.getRawParameterValue("dry-wet") / 100;
	float curFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	bool loop = *tree.getRawParameterValue("loop") >= 1;
	// process each channel
	for (int channel = 0;channel < numInputChannels;channel++)
	{
		// get parameters/objects to use when processing the channel
		float* channelData = buffer.getWritePointer(channel);
		CircularBuffer* delayBuf = channel == 0 ? &leftBuffer : &rightBuffer;
		DelayAmp* amps = channel == 0 ? leftAmps : rightAmps;
		Filter* filters = channel == 0 ? leftFilters : rightFilters;
		float dryWet = lastDryWet;
		float dryWetStep;
		if (juce::approximatelyEqual(lastDryWet, curDryWet))
			dryWetStep = 0;
		else
			dryWetStep = (curDryWet - lastDryWet) / numSamples;
		// add dry signal to temporary buffer, will add to delay buffer later
		for (size_t i = 0;i < numSamples;i++)
			tempBuffer[i] = channelData[i];
		// scale the dry signal by the wet/dry and its amplitude slider
		float dryAmp;
		float dryAmpStep;
		float dryAmpStart = amps[0].getLastValue();
		float dryAmpEnd = amps[0].getCurrentValue();
		if (juce::approximatelyEqual(dryAmpStart, dryAmpEnd))
		{
			dryAmp = dryAmpStart;
			dryAmpStep = (dryAmpEnd - dryAmpStart) / numSamples;
		}
		else
		{
			dryAmp = dryAmpStart;
			dryAmpStep = 0;
		}
		for (size_t i = 0;i < numSamples;i++)
		{
			dryAmp += dryAmpStep;
			dryWet += dryWetStep;
			channelData[i] *= (1 - dryWet) * dryAmp;
		}
		// if enabled, add looped signal to output and temporary buffer for
		// adding back to the delay buffer
		if (loop || lastLoop)
		{
			size_t loopDelay = delay * intervals - numSamples;
			delayBuf->applyGainToSamples(
				loopDelay, numSamples, lastFalloff, curFalloff
			);
			delayBuf->applyFilterToSamples(loopDelay, numSamples, &filters[0]);
			dryAmpStart *= lastDryWet;
			dryAmpEnd *= curDryWet;
			if (!lastLoop)
				dryAmpStart = 0;
			else if (!loop || fadeOut)
				dryAmpEnd = 0;
			delayBuf->sumWithSamplesRamped(
				loopDelay, channelData, numSamples, dryAmpStart, dryAmpEnd
			);
			float* tmp = tempBuffer.data();
			float feedbackStart = lastLoop ? 1 : 0;
			float feedbackEnd = loop ? 1 : 0;
			delayBuf->sumWithSamplesRamped(
				loopDelay, tmp, numSamples, feedbackStart, feedbackEnd
			);
		}
		// add dry (and looped, if enabled) signal to the delay buffer
		if (!lastBlockFadeOut)
			delayBuf->addSamples(tempBuffer.data(), numSamples);
		else if (!fadeOut)
			delayBuf->addSamplesRamped(tempBuffer.data(), numSamples);
		// skip wet signal if delay is currently changing
		if (fadeOut && lastBlockFadeOut)
			continue;
		// get wet signal scaled by amplitude sliders and falloff value
		for (size_t i = 0;i < numSamples;i++)
			tempBuffer[i] = 0;
		for (size_t i = intervals - 1;i > 0;i--)
		{
			size_t d = delay * i;
			delayBuf->applyGainToSamples(
				d, numSamples, lastFalloff, curFalloff
			);
			delayBuf->applyFilterToSamples(d, numSamples, &filters[i]);
			float* data = tempBuffer.data();
			if (amps[i].hasNewValue())
			{
				float g1 = amps[i].getLastValue();
				float g2 = amps[i].getCurrentValue();
				delayBuf->sumWithSamplesRamped(d, data, numSamples, g1, g2);
			}
			else
			{
				float gain = amps[i].getCurrentValue();
				delayBuf->sumWithSamples(d, data, numSamples, gain);
			}
		}
		// scale wet signal by dry/wet, fade out if delay time has started
		// changing on this block, and add to output signal
		dryWet = lastDryWet;
		float fade = 1;
		float fadeStep = 1.0f / numSamples;
		for (size_t i = 0;i < numSamples;i++)
		{
			fade -= fadeStep;
			dryWet += dryWetStep;
			tempBuffer[i] *= dryWet * (fadeOut ? fade : 1);
			channelData[i] += tempBuffer[i];
		}
		// clear buffer if the delay time has changed
		if (fadeOut)
			delayBuf->clear();
	}
	// save parameters to compare with next block
	lastDelay = curDelay;
	lastIntervals = curIntervals;
	lastBlockFadeOut = fadeOut;
	lastDryWet = curDryWet;
	lastFalloff = curFalloff;
	lastLoop = loop;
}

// === Operations =========================================================
void PluginProcessor::resetLeftAmps()
{
	for (size_t i = 0;i < maxIntervals;i++)
	{
		std::string id = getIdForLeftIntervalAmp((int) i);
		juce::RangedAudioParameter* param = tree.getParameter(id);
		param->beginChangeGesture();
		param->setValueNotifyingHost(param->getDefaultValue());
		param->endChangeGesture();
	}
}

void PluginProcessor::resetRightAmps()
{
	for (size_t i = 0;i < maxIntervals;i++)
	{
		std::string id = getIdForRightIntervalAmp((int) i);
		juce::RangedAudioParameter* param = tree.getParameter(id);
		param->beginChangeGesture();
		param->setValueNotifyingHost(param->getDefaultValue());
		param->endChangeGesture();
	}
}

void PluginProcessor::copyLeftAmpsToRight()
{
	for (size_t i = 0;i < maxIntervals;i++)
	{
		std::string leftId = getIdForLeftIntervalAmp((int) i);
		juce::RangedAudioParameter* left = tree.getParameter(leftId);
		std::string rightId = getIdForRightIntervalAmp((int) i);
		juce::RangedAudioParameter* right = tree.getParameter(rightId);
		right->beginChangeGesture();
		right->setValueNotifyingHost(left->getValue());
		right->endChangeGesture();
	}
}

void PluginProcessor::copyRightAmpsToLeft()
{
	for (size_t i = 0;i < maxIntervals;i++)
	{
		std::string leftId = getIdForLeftIntervalAmp((int) i);
		juce::RangedAudioParameter* left = tree.getParameter(leftId);
		std::string rightId = getIdForRightIntervalAmp((int) i);
		juce::RangedAudioParameter* right = tree.getParameter(rightId);
		left->beginChangeGesture();
		left->setValueNotifyingHost(right->getValue());
		left->endChangeGesture();
	}
}

void PluginProcessor::linkFilters()
{
	std::string high = "left-high-pass";
	std::string low = "left-low-pass";
	std::string mix = "left-filter-mix";
	for (size_t i = 0;i < maxIntervals;i++)
	{
		rightFilters[i].attachToParameters(&tree, high, low, mix);
	}
}

void PluginProcessor::unlinkFilters()
{
	std::string high = "right-high-pass";
	std::string low = "right-low-pass";
	std::string mix = "right-filter-mix";
	for (size_t i = 0;i < maxIntervals;i++)
	{
		rightFilters[i].attachToParameters(&tree, high, low, mix);
	}
}

void PluginProcessor::linkDelays()
{
	for (int i = 0;i < maxIntervals;i++)
	{
		rightAmps[i].listenTo(&tree, getIdForLeftIntervalAmp(i));
	}
}

void PluginProcessor::unlinkDelays()
{
	for (int i = 0;i < maxIntervals;i++)
	{
		rightAmps[i].listenTo(&tree, getIdForRightIntervalAmp(i));
	}
}

void PluginProcessor::notifyHostOfStateChange()
{
	updateHostDisplay(ChangeDetails().withParameterInfoChanged(true));
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
	{
		tree.replaceState(juce::ValueTree::fromXml(*xml));
		notifyHostOfStateChange();
	}
}

// === Private Helper =========================================================
size_t PluginProcessor::getDelaySamples()
{
	float ms = *tree.getRawParameterValue("delay-time");
	return (size_t) (lastSampleRate * ms / 1000);
}

size_t PluginProcessor::getCurrentNumIntervals()
{
	return (size_t) *tree.getRawParameterValue("num-intervals");
}