#include "CircularBuffer.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include "Filter.h"

CircularBuffer::CircularBuffer() : CircularBuffer(1024) { }

CircularBuffer::CircularBuffer(size_t capacity) : sampleCount(0)
{
    resize(capacity);
}

void CircularBuffer::addSample(const float sample)
{
    buffer[capacityMask(sampleCount++)] = sample;
}

void CircularBuffer::addSamples(const float* samples, size_t numToAdd)
{
    jassert(numToAdd <= buffer.size());

    size_t startSample = capacityMask(sampleCount);
    size_t spaceBeforeWrap = buffer.size() - startSample;
    size_t numPreWrap = juce::jmin(numToAdd, spaceBeforeWrap);
    size_t numPostWrap = numToAdd - numPreWrap;

    memcpy(buffer.data() + startSample, samples, numPreWrap * sizeof(float));
    memcpy(buffer.data(), samples + numPreWrap, numPostWrap * sizeof(float));

    sampleCount += numToAdd;
}

void CircularBuffer::addSamplesRamped(const float* samples, size_t numToAdd)
{
    // sampleCount changes as a result of addSamples, so save its value here
    size_t startSample = capacityMask(sampleCount);
    addSamples(samples, numToAdd);

    float gain = 0;
    float gainStep = 1.0f / numToAdd;
    size_t numPreWrap = juce::jmin(numToAdd, buffer.size() - startSample);
    size_t numPostWrap = numToAdd - numPreWrap;

    for (size_t i = startSample;i < numPreWrap;i++)
    {
        gain += gainStep;
        buffer[i] *= gain;
    }
    for (size_t i = 0;i < numPostWrap;i++)
    {
        gain *= gainStep;
        buffer[i] *= gain;
    }
}

float CircularBuffer::getSample(size_t delay)
{
    return buffer[capacityMask(sampleCount - 1 - delay)];
}

void CircularBuffer::getSamples(size_t delay, float* output, size_t length)
{
    jassert(length <= buffer.size());

    size_t start = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - start);
    size_t numPostWrap = length - numPreWrap;

    memcpy(output, buffer.data() + start, numPreWrap * sizeof(float));
    memcpy(output + numPreWrap, buffer.data(), numPostWrap * sizeof(float));
}

void CircularBuffer::sumWithSamples(size_t delay, float* output, size_t length,
    float gain)
{
    if (juce::approximatelyEqual(gain, 0.0f))
    {
        return;
    }

    size_t start = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - start);
    size_t numPostWrap = length - numPreWrap;

    for (size_t i = 0;i < numPreWrap;i++)
    {
        output[i] += buffer[i++ + start] * gain;
    }
    for (size_t i = 0;i < numPostWrap;i++)
    {
        output[i + numPreWrap] += buffer[i++] * gain;
    }
}

void CircularBuffer::sumWithSamplesRamped(size_t delay, float* output,
    size_t length, float startGain, float endGain)
{
    if (juce::approximatelyEqual(startGain, endGain))
    {
        sumWithSamples(delay, output, length, startGain);
        return;
    }

    size_t start = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - start);
    size_t numPostWrap = length - numPreWrap;
    float gain = startGain;
    float gainStep = (endGain - startGain) / length;

    for (size_t i = 0;i < numPreWrap;i++)
    {
        gain += gainStep;
        output[i] += buffer[i++ + start] * gain;
    }
    for (size_t i = 0;i < numPostWrap;i++)
    {
        gain += gainStep;
        output[i + numPreWrap] += buffer[i++] * gain;
    }
}

void CircularBuffer::applyGainToSamples(size_t delay, size_t length,
    float gain)
{
    if (juce::approximatelyEqual(gain, 1.0f))
    {
        return;
    }

    size_t start = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - start);
    size_t numPostWrap = length - numPreWrap;

    for (size_t i = 0;i < numPreWrap;i++)
    {
        buffer[start + i] *= gain;
    }
    for (size_t i = 0;i < numPostWrap;i++)
    {
        buffer[i] *= gain;
    }
}

void CircularBuffer::applyGainToSamples(size_t delay, size_t length,
    float startGain, float endGain)
{
    if (juce::approximatelyEqual(startGain, endGain))
    {
        applyGainToSamples(delay, length, startGain);
        return;
    }

    size_t startSample = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - startSample);
    size_t numPostWrap = length - numPreWrap;
    float gain = startGain;
    float gainStep = (endGain - startGain) / length;

    for (size_t i = 0;i < numPreWrap;i++)
    {
        gain += gainStep;
        buffer[startSample + i] *= gain;
    }
    for (size_t i = 0;i < numPostWrap;i++)
    {
        gain += gainStep;
        buffer[i] *= gain;
    }
}

void CircularBuffer::applyFilterToSamples(size_t delay, size_t length,
    Filter* filter)
{
    size_t start = capacityMask(sampleCount - delay - length);
    size_t numPreWrap = juce::jmin(length, buffer.size() - start);
    size_t numPostWrap = length - numPreWrap;

    filter->processSamples(buffer.data() + start, numPreWrap);
    filter->processSamples(buffer.data(), numPostWrap);
}

void CircularBuffer::clear()
{
    sampleCount = 0;
    memset(buffer.data(), 0, buffer.size() * sizeof(float));
}

void CircularBuffer::resize(size_t newLength)
{
    jassert(juce::isPowerOfTwo(newLength) && newLength >= 2);
    buffer.resize(newLength);
    clear();
}

void CircularBuffer::resize(double sampleRate, float maxDelaySeconds)
{
    int maxSamples = static_cast<int>(std::ceil(sampleRate * maxDelaySeconds));
    size_t newLength = static_cast<size_t>(juce::nextPowerOfTwo(maxSamples));
    resize(newLength);
}

size_t CircularBuffer::capacityMask(size_t sample)
{
    return sample & (buffer.size() - 1);
}