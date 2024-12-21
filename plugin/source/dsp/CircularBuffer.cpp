#include "CircularBuffer.h"
#include <algorithm>
#include <stdexcept>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Filter.h"

// === Lifecycle ==============================================================
CircularBuffer::CircularBuffer() : CircularBuffer(44100) { }

CircularBuffer::CircularBuffer(size_t capacity)
    : buffer(capacity), leastRecentSample(0), numSamples(0)
{ 
    if (capacity == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
}

// === Add Samples ============================================================
void CircularBuffer::addSample(const float sample)
{
    leastRecentSample = (leastRecentSample + 1) % buffer.size();
    buffer[leastRecentSample] = sample;
    if (numSamples < buffer.size())
        numSamples++;
}

void CircularBuffer::addSamples(const float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many samples");
    for (size_t i = 0;i < length;i++)
    {
        leastRecentSample = (leastRecentSample + 1) % buffer.size();
        buffer[leastRecentSample] = samples[i];
    }
    numSamples += length;
    if (numSamples > buffer.size())
        numSamples = buffer.size();
}

void CircularBuffer::addSamplesRamped(const float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many samples");
    for (size_t i = 0;i < length;i++)
    {
        float gain = (i + 1) / (float) length;
        leastRecentSample = (leastRecentSample + 1) % buffer.size();
        buffer[leastRecentSample] = samples[i] * gain;
    }
    numSamples += length;
    if (numSamples > buffer.size())
        numSamples = buffer.size();
}

// === Get Samples ============================================================
float CircularBuffer::getSample(size_t delay)
{
    if (delay > buffer.size())
        throw std::invalid_argument("Delay value too long");
    if (delay >= numSamples)
        return 0;
    if (leastRecentSample >= delay)
        return buffer[leastRecentSample - delay];
    else
        return buffer[buffer.size() + leastRecentSample - delay];
}

void CircularBuffer::getSamples
(size_t delay, float* output, size_t len)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    for (size_t i = 0;i < len;i++)
    {
        size_t curDelay = delay + len - 1 - i;
        if (curDelay >= numSamples)
            output[i] = 0;
        if (leastRecentSample >= curDelay)
            output[i] = buffer[leastRecentSample - curDelay];
        else
            output[i] = buffer[buffer.size() + leastRecentSample - curDelay];
    }
}

void CircularBuffer::sumWithSamples
(size_t delay, float* output, size_t len, float gain)
{
    if (juce::approximatelyEqual(gain, 0.0f))
        return;
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    for (size_t i = 0;i < len;i++)
    {
        size_t curDelay = delay + len - 1 - i;
        if (curDelay >= numSamples)
            continue;
        if (leastRecentSample >= curDelay)
        {
            output[i] += buffer[leastRecentSample - curDelay] * gain;
        }
        else
        {
            float s = buffer[buffer.size() + leastRecentSample - curDelay];
            output[i] += s * gain;
        }
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
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    float gain = start;
    float gainStep = (end - start) / len;
    for (size_t i = 0;i < len;i++)
    {
        size_t curDelay = delay + len - 1 - i;
        if (curDelay >= numSamples)
            continue;
        if (leastRecentSample >= curDelay)
        {
            output[i] += buffer[leastRecentSample - curDelay] * gain;
        }
        else
        {
            float s = buffer[buffer.size() + leastRecentSample - curDelay];
            output[i] += s * gain;
        }
        gain += gainStep;
    }
}


// === Manipulate Samples =====================================================
void CircularBuffer::applyGainToSamples(size_t delay, size_t len, float gain)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    if (juce::approximatelyEqual(gain, 1.0f))
        return;
    size_t index = leastRecentSample - delay;
    if (index > buffer.size())
        index += buffer.size(); // if index underflowed, overflow it back
    for (size_t i = 0;i < len;i++)
    {
        buffer[index] *= gain;
        index = (index + 1) % buffer.size();
    }
}

void CircularBuffer::applyGainToSamples
(size_t delay, size_t len, float start, float end)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    bool startIsOne = juce::approximatelyEqual(start, 1.0f);
    bool endIsOne = juce::approximatelyEqual(end, 1.0f);
    if (startIsOne && endIsOne)
        return;
    if (juce::approximatelyEqual(start, end))
    {
        applyGainToSamples(delay, len, start);
        return;
    }
    size_t index = leastRecentSample - delay;
    if (index > buffer.size())
        index += buffer.size(); // if index underflowed, overflow it back
    float gainStep = (end - start) / len;
    for (size_t i = 0;i < len;i++)
    {
        start += gainStep;
        buffer[index] *= start;
        index = (index + 1) % buffer.size();
    }
}

void CircularBuffer::applyFilterToSamples
(size_t delay, size_t len, Filter* filter)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    size_t index = leastRecentSample - delay;
    if (index > buffer.size())
        index += buffer.size(); // if index underflowed, overflow it back
    for (size_t i = 0;i < len;i++)
    {
        buffer[index] = filter->processSample(buffer[index]);
        index = (index + 1) % buffer.size();
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
    if (newLength == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
    buffer.resize(newLength);
    clear();
}

void CircularBuffer::resize(double sampleRate, float maxDelaySeconds)
{
    double maxSamples = sampleRate * maxDelaySeconds;
    size_t newLength = (size_t) std::ceil(maxSamples);
    resize(newLength);
}