#include "PluginProcessor.h"
#include <algorithm>
#include <juce_dsp/juce_dsp.h>
#include "PluginEditor.h"
#include "ParameterFactory.h"

const float PluginProcessor::maxDelayTime = 250;

const NoteValue PluginProcessor::noteValues[numNoteValues] = {
	{ "16th triplet", 0.0417f },
	{ "16th", 0.0625f },
	{ "16th dotted", 0.09375f },
	{ "8th triplet", 0.0833f },
	{ "8th", 0.125f },
	{ "8th dotted", 0.1875f }
};

PluginProcessor::PluginProcessor() :
	AudioProcessor(createBusesProperties()),
	tree(*this, nullptr, "PARAMETERS", createParameters()),
	lastSampleRate(44100),
	lastBpm(-1)
{
	for (int i = 0;i < maxIntervals;i++)
	{
		leftAmps[i].listenTo(&tree, getIdForLeftIntervalAmp(i));
		rightAmps[i].listenTo(&tree, getIdForRightIntervalAmp(i));

		leftFilters[i].attachToParameters(&tree, "left-high-pass",
			"left-low-pass", "left-filter-mix");
		rightFilters[i].attachToParameters(&tree, "right-high-pass",
			"right-low-pass", "right-filter-mix");
	}

	updateParametersOnReset();

#if PERFETTO
	MelatoninPerfetto::get().beginSession();
#endif
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

	parameters.add(ParameterFactory::createTimeParameter("delay-time",
		"Delay Time", 20, maxDelayTime, 1, 100));
	
	juce::StringArray noteOptions;
	for (size_t i = 0;i < numNoteValues;i++)
	{
		noteOptions.add(noteValues[i].name);
	}

	parameters.add(ParameterFactory::createChoiceParameter("delay-time-sync",
		"Delay Time (Rhythmic)", noteOptions, 4));
	parameters.add(ParameterFactory::createBoolParameter("tempo-sync",
		"Tempo Sync", "ON", "OFF", 0));

	parameters.add(ParameterFactory::createIntParameter("num-intervals",
		"Intervals", 8, 16, 8));
	parameters.add(ParameterFactory::createBoolParameter("loop", "Loop", "ON",
		"OFF", 0));

	parameters.add(ParameterFactory::createBoolParameter("delays-linked",
		"Delays Mirrored", "ON", "OFF", 0));
	parameters.add(ParameterFactory::createBoolParameter("filters-linked",
		"Filters Mirrored", "ON", "OFF", 0));

	parameters.add(ParameterFactory::createPercentageParameter("wet",
		"Wet Mix", 100));
	parameters.add(ParameterFactory::createPercentageParameter("falloff",
		"Auto-Falloff", 0));

	parameters.add(ParameterFactory::createFreqParameter("left-high-pass",
		"Filter Left Low", 20));
	parameters.add(ParameterFactory::createFreqParameter("left-low-pass",
		"Filter Left High", 20000));
	parameters.add(ParameterFactory::createPercentageParameter(
		"left-filter-mix", "Filter Left Mix", 100));
	parameters.add(ParameterFactory::createFreqParameter("right-high-pass",
		"Filter Right Low", 20));
	parameters.add(ParameterFactory::createFreqParameter("right-low-pass",
		"Filter Right High", 20000));
	parameters.add(ParameterFactory::createPercentageParameter(
		"right-filter-mix", "Filter Right Mix", 100));
		
	for (int i = 0;i < maxIntervals;i++)
	{
		std::string leftName;
		std::string rightName;
		float lDefault = i == 0 ? 1 : 0;
		float rDefault = i == 0 ? 1 : 0;

		if (i == 0)
		{
			leftName = "Pre-Repeat Amplitude L";
			rightName = "Pre-Repeat Amplitude R";
		}
		else 
		{
			std::string number = std::to_string(i);
			if (i < 10)
			{
				number = "0" + number;
			}
			leftName = "Repeat " + number + " Amplitude L";
			rightName = "Repeat " + number + " Amplitude R";
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

juce::AudioProcessor::BusesProperties PluginProcessor::createBusesProperties()
{
	juce::AudioProcessor::BusesProperties props;
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
	props = props.withInput("Input", juce::AudioChannelSet::stereo(), true);
#endif
	props = props.withOutput("Output", juce::AudioChannelSet::stereo(), true);
#endif
	return props;
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
	{
		return false;
	}

#if !JucePlugin_IsSynth
	// Check if the input matches the output
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
	{
		return false;
	}
#endif
	return true;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
	return maxIntervals * maxDelayTime / 1000;
}

float PluginProcessor::getSecondsForNoteValue(int index)
{
	if (index >= static_cast<int>(numNoteValues) || index < 0 || lastBpm < 0)
	{
		return 0;
	}
	return (noteValues[index].proportion / static_cast<float>(lastBpm)) * 240;
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = static_cast<unsigned>(samplesPerBlock);
	spec.numChannels = static_cast<unsigned>(getTotalNumOutputChannels());
	for (int i = 0;i < maxIntervals;i++)
	{
		leftFilters[i].prepare(spec);
		rightFilters[i].prepare(spec);
	}

	leftBuffer.resize(sampleRate, (maxDelayTime / 1000) * (maxIntervals + 1));
	rightBuffer.resize(sampleRate, (maxDelayTime / 1000) * (maxIntervals + 1));
	tempBuffer.resize((size_t) samplesPerBlock, 0.0f);
		
	lastSampleRate = sampleRate;

	lastAmpsLinked = *tree.getRawParameterValue("delays-linked") >= 1;
	lastFiltersLinked = *tree.getRawParameterValue("filters-linked") >= 1;

	updateParametersOnReset();
}

void PluginProcessor::releaseResources() { }

void PluginProcessor::processBlock
(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
	TRACE_DSP();
	juce::ignoreUnused(midiMessages); // not a midi plugin

	int numInputChannels = getTotalNumInputChannels();
	int numOutputChannels = getTotalNumOutputChannels();
	for (int i = numInputChannels;i < numOutputChannels;i++)
	{
		buffer.clear(i, 0, buffer.getNumSamples());
	}
	
	numSamples = static_cast<size_t>(buffer.getNumSamples());	
	
	handleTempoSync();
	handleParameterLinking();
	updateCurrentBlockParameters();

	float* leftAudio = buffer.getWritePointer(0);
	processChannel(leftAudio, &leftBuffer, leftAmps, leftFilters);
	float* rightAudio = buffer.getWritePointer(1);
	processChannel(rightAudio, &rightBuffer, rightAmps, rightFilters);
	
	updateLastBlockParameters();
}

void PluginProcessor::handleTempoSync()
{
	juce::AudioPlayHead* playhead = getPlayHead();
	if (playhead && playhead->getPosition().hasValue())
	{
		lastBpm = playhead->getPosition()->getBpm().orFallback(120.0);
	}
}

void PluginProcessor::handleParameterLinking()
{
	bool ampsLinked = *tree.getRawParameterValue("delays-linked") >= 1;
	if (ampsLinked && !lastAmpsLinked)
	{
		linkDelays();
	}
	else if (!ampsLinked && lastAmpsLinked)
	{
		unlinkDelays();
	}
	lastAmpsLinked = ampsLinked;

	bool filtersLinked = *tree.getRawParameterValue("filters-linked") >= 1;
	if (filtersLinked && !lastFiltersLinked)
	{
		linkFilters();
	}
	else if (!filtersLinked && lastFiltersLinked)
	{
		unlinkFilters();
	}
	lastFiltersLinked = filtersLinked;
}

void PluginProcessor::updateCurrentBlockParameters()
{
	currentDelay = getDelaySamples();
	currentNumIntervals = getCurrentNumIntervals();

	bool delayChanged = currentDelay != lastBlockDelay;
	bool intervalsChanged = currentNumIntervals != lastBlockNumIntervals;
	fadeOut = delayChanged || intervalsChanged;

	currentWet = *tree.getRawParameterValue("wet") / 100;
	currentFalloff = 1 - (*tree.getRawParameterValue("falloff") / 100);
	loop = *tree.getRawParameterValue("loop") >= 1;
}

void PluginProcessor::updateLastBlockParameters()
{
	lastBlockDelay = currentDelay;
	lastBlockNumIntervals = currentNumIntervals;
	lastBlockFadeOut = fadeOut;
	lastBlockWet = currentWet;
	lastBlockFalloff = currentFalloff;
	lastBlockLoop = loop;
}

void PluginProcessor::updateParametersOnReset()
{
	updateCurrentBlockParameters();
	fadeOut = false;
	updateLastBlockParameters();
}

void PluginProcessor::processChannel(float* audio, CircularBuffer* buffer,
	DelayAmp* amps, Filter* filters)
{
	memcpy(tempBuffer.data(), audio, numSamples * sizeof(float));

	float dryAmpStart = amps[0].getLastValue();
	float dryAmpEnd = amps[0].getCurrentValue();
	processDrySignal(audio, dryAmpStart, dryAmpEnd);
	processLoopedSignal(audio, buffer, dryAmpStart, dryAmpEnd, filters);

	if (!lastBlockFadeOut)
	{
		buffer->addSamples(tempBuffer.data(), numSamples);
	}
	else if (!fadeOut)
	{
		buffer->addSamplesRamped(tempBuffer.data(), numSamples);
	}
	else
	{
		return;
	}

	processWetSignal(audio, buffer, amps, filters);

	if (fadeOut)
	{
		buffer->clear();
		for (size_t i = 0;i < maxIntervals;i++)
		{
			filters[i].reset();
		}
	}
}


void PluginProcessor::processDrySignal(float* audio, float ampStart,
	float ampEnd)
{
	float step = (ampEnd - ampStart) / numSamples;
	for (size_t i = 0;i < numSamples;i++)
	{
		ampStart += step;
		audio[i] *= ampStart;
	}
}

void PluginProcessor::processLoopedSignal(float* audio, CircularBuffer* buffer,
	float ampStart, float ampEnd, Filter* filters)
{
	if (!loop && !lastBlockLoop)
	{
		return;
	}

	size_t loopDelay = lastBlockDelay * lastBlockNumIntervals - numSamples;
	buffer->applyGainToSamples(loopDelay, numSamples, lastBlockFalloff,
		currentFalloff);
	buffer->applyFilterToSamples(loopDelay, numSamples, &filters[0]);

	float feedbackStart = lastBlockLoop ? 1 : 0;
	float feedbackEnd = loop ? 1 : 0;
	buffer->sumWithSamplesRamped(loopDelay, tempBuffer.data(), numSamples,
		feedbackStart, feedbackEnd);

	feedbackStart *= lastBlockWet * ampStart;
	feedbackEnd *= currentWet * ampEnd;
	buffer->sumWithSamplesRamped(loopDelay, audio, numSamples, feedbackStart,
		feedbackEnd);
}

void PluginProcessor::processWetSignal(float* audio, CircularBuffer* buffer,
	DelayAmp* amps, Filter* filters)
{
	memset(tempBuffer.data(), 0, tempBuffer.size() * sizeof(float));

	for (size_t i = lastBlockNumIntervals - 1;i > 0;i--)
	{
		size_t intervalDelay = lastBlockDelay * i;
		buffer->applyGainToSamples(intervalDelay, numSamples, lastBlockFalloff,
			currentFalloff);
		buffer->applyFilterToSamples(intervalDelay, numSamples, &filters[i]);

		float g1 = amps[i].getLastValue();
		float g2 = amps[i].getCurrentValue();
		buffer->sumWithSamplesRamped(intervalDelay, tempBuffer.data(),
			numSamples, g1, g2);
	}

	float wet = lastBlockWet;
	float wetStep = (currentWet - lastBlockWet) / numSamples;
	float fade = 1;
	float fadeStep = fadeOut ? (1.0f / numSamples) : 0;

	for (size_t i = 0;i < numSamples;i++)
	{
		fade -= fadeStep;
		wet += wetStep;
		audio[i] += tempBuffer[i] * wet * fade;
	}
}

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

void PluginProcessor::notifyHostOfStateChange()
{
	updateHostDisplay(ChangeDetails().withParameterInfoChanged(true));
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
	return new PluginProcessor();
}

juce::AudioProcessorEditor *PluginProcessor::createEditor()
{
	return new PluginEditor(*this);
}

size_t PluginProcessor::getDelaySamples()
{
	size_t result;

	if (*tree.getRawParameterValue("tempo-sync") >= 1)
	{
		float noteIndex = *tree.getRawParameterValue("delay-time-sync");
		float sec = getSecondsForNoteValue((int) noteIndex);
		result = static_cast<size_t>(lastSampleRate * sec);
	}
	else
	{
		float ms = *tree.getRawParameterValue("delay-time");
		result = static_cast<size_t>(lastSampleRate * ms / 1000);
	}
	
	size_t limit = static_cast<size_t>((maxDelayTime / 1000) * lastSampleRate);
	return juce::jmin(result, limit);
}

size_t PluginProcessor::getCurrentNumIntervals()
{
	return static_cast<size_t>(*tree.getRawParameterValue("num-intervals"));
}