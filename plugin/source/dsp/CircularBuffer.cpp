#include "CircularBuffer.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include "Filter.h"

// === Lifecycle ==============================================================
CircularBuffer::CircularBuffer() : CircularBuffer(44100) { }

CircularBuffer::CircularBuffer(size_t capacity)
    : buffer(juce::jmax(capacity, 16UL)), leastRecentSample(0), numSamples(0)
{ }

// === Add Samples ============================================================
void CircularBuffer::addSample(const float sample)
{
    buffer[leastRecentSample] = sample;
    leastRecentSample++;
    if (leastRecentSample == buffer.size())
        leastRecentSample = 0;
    if (numSamples < buffer.size())
        numSamples++;
}

void CircularBuffer::addSamples(const float* samples, size_t length)
{
    length = juce::jmin(length, buffer.size());
    size_t limit = juce::jmin(leastRecentSample + length, buffer.size());
    size_t srcIndex = 0;
    for (size_t i = leastRecentSample;i < limit;i++)
        buffer[i] = samples[srcIndex++];
    // wrap around to the beginning of the buffer if necessary
    for (size_t i = 0;srcIndex < length;i++)
        buffer[i] = samples[srcIndex++];
    numSamples = juce::jmin(numSamples + length, buffer.size());
    leastRecentSample = (leastRecentSample + length) % buffer.size();
}

void CircularBuffer::addSamplesRamped(const float* samples, size_t length)
{
    length = juce::jmin(length, buffer.size());
    size_t limit = juce::jmin(leastRecentSample + length, buffer.size());
    size_t srcIndex = 0;
    float gain = 0;
    float gainStep = 1.0f / length;
    for (size_t i = leastRecentSample;i < limit;i++)
    {
        gain += gainStep;
        buffer[i] = samples[srcIndex++] * gain;
    }
    // wrap around to the beginning of the buffer if necessary
    for (size_t i = 0;srcIndex < length;i++)
    {
        gain += gainStep;
        buffer[i] = samples[srcIndex++] * gain;
    }
    numSamples = juce::jmin(numSamples + length, buffer.size());
    leastRecentSample = (leastRecentSample + length) % buffer.size();
}

// === Get Samples ============================================================
float CircularBuffer::getSample(size_t delay)
{
    if (delay >= numSamples)
        return 0;
    size_t index = leastRecentSample - 1 - delay;
    if (index > buffer.size())
        index += buffer.size(); // correct underflow
    return buffer[index];
}

void CircularBuffer::getSamples
(size_t delay, float* output, size_t len)
{
    // setup counters and boundaries
    size_t i = leastRecentSample - delay - len;
    if (i > buffer.size())
        i += buffer.size(); // correct underflow
    size_t limit = juce::jmin(i + len, buffer.size());
    size_t destIndex = 0;
    // loop through the samples
    for (;i < limit;i++)
    {
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] = s;
    }
    // wrap around to the beginning of the buffer if necessary
    for (i = 0;destIndex < len;i++)
    {
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] = s;
    }
}

void CircularBuffer::sumWithSamples
(size_t delay, float* output, size_t len, float gain)
{
    if (juce::approximatelyEqual(gain, 0.0f))
        return;
    // setup counters and boundaries
    size_t i = leastRecentSample - delay - len;
    if (i > buffer.size())
        i += buffer.size(); // correct underflow
    size_t limit = juce::jmin(i + len, buffer.size());
    size_t destIndex = 0;
    // loop through the samples
    for (;i < limit;i++)
    {
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] += s * gain;
    }
    // wrap around to the beginning of the buffer if necessary
    for (i = 0;destIndex < len;i++)
    {
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] += s * gain;
    }
}

void CircularBuffer::sumWithSamplesRamped
(size_t delay, float* output, size_t len, float start, float end)
{
    if (juce::approximatelyEqual(start, end))
    {
        sumWithSamples(delay, output, len, start);
        return;
    }
    // setup counters and boundaries
    size_t i = leastRecentSample - delay - len;
    if (i > buffer.size())
        i += buffer.size(); // correct underflow
    size_t limit = juce::jmin(i + len, buffer.size());
    size_t destIndex = 0;
    float gain = start;
    float gainStep = (end - start) / len;
    // loop through the samples
    for (;i < limit;i++)
    {
        gain += gainStep;
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] += s * gain;
    }
    // wrap around to the beginning of the buffer if necessary
    for (i = 0;destIndex < len;i++)
    {
        gain += gainStep;
        float s = delay + len - destIndex <= numSamples ? buffer[i] : 0;
        output[destIndex++] += s * gain;
    }
}

// === Manipulate Samples =====================================================
void CircularBuffer::applyGainToSamples(size_t delay, size_t len, float gain)
{
    if (delay > numSamples || juce::approximatelyEqual(gain, 1.0f))
        return;
    // setup counters and boundaries
    size_t start = leastRecentSample - delay - len;
    if (start > buffer.size())
        start += buffer.size(); // correct underflow
    size_t limit = juce::jmin(start + len, buffer.size());
    // loop through the samples
    for (size_t i = start;i < limit;i++)
        buffer[i] *= gain;
    // wrap around to the beginning of the buffer if necessary
    size_t wrap = start + len - limit;
    for (size_t i = 0;i < wrap;i++)
        buffer[i] *= gain;
}

void CircularBuffer::applyGainToSamples
(size_t delay, size_t len, float start, float end)
{
    bool startIsOne = juce::approximatelyEqual(start, 1.0f);
    bool endIsOne = juce::approximatelyEqual(end, 1.0f);
    if (startIsOne && endIsOne)
        return;
    if (juce::approximatelyEqual(start, end))
    {
        applyGainToSamples(delay, len, start);
        return;
    }
    // setup counters and boundaries
    size_t startIndex = leastRecentSample - delay - len;
    if (startIndex > buffer.size())
        startIndex += buffer.size(); // correct underflow
    size_t limit = juce::jmin(startIndex + len, buffer.size());
    float gainStep = (end - start) / len;
    // loop through the samples
    for (size_t i = startIndex;i < limit;i++)
    {
        start += gainStep;
        buffer[i] *= start;
    }
    // wrap around to the beginning of the buffer if necessary
    size_t wrap = startIndex + len - limit;
    for (size_t i = 0;i < wrap;i++)
    {
        start += gainStep;
        buffer[i] *= start;
    }
}

// func is a little less elegant that others but it's more efficient this way
void CircularBuffer::applyFilterToSamples
(size_t delay, size_t len, Filter* filter)
{
    // get bounds of area to process
    size_t start = leastRecentSample - delay - len;
    size_t end = start + len;
    // handle underflow in bounds
    if (start > buffer.size())
        start += buffer.size();
    if (end > buffer.size())
        end += buffer.size();
    // process samples
    if (start > end && end != 0)
    {
        filter->processSamples(buffer.data() + start, buffer.size() - start);
        filter->processSamples(buffer.data(), end);
    }
    else
    {
        filter->processSamples(buffer.data() + start, len);
    }
}

// === Other Operations =======================================================
void CircularBuffer::clear()
{
    leastRecentSample = 0;
    numSamples = 0;
}

void CircularBuffer::resize(size_t newLength)
{
    buffer.resize(juce::jmax(newLength, 16UL));
    clear();
}

void CircularBuffer::resize(double sampleRate, float maxDelaySeconds)
{
    double maxSamples = sampleRate * maxDelaySeconds;
    size_t newLength = (size_t) std::ceil(maxSamples);
    resize(newLength);
}