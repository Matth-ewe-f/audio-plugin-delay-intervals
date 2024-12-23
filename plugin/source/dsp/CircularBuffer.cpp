#include "CircularBuffer.h"
#include <algorithm>
#include <stdexcept>
#include <juce_audio_basics/juce_audio_basics.h>
#include <melatonin_perfetto/melatonin_perfetto.h>
#include "Filter.h"

// === Lifecycle ==============================================================
CircularBuffer::CircularBuffer() : CircularBuffer(44100) { }

CircularBuffer::CircularBuffer(size_t capacity)
    : buffer(capacity), mostRecentSample(0), numSamples(0)
{ 
    if (capacity == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
}

// === Add Samples ============================================================
void CircularBuffer::addSample(const float sample)
{
    mostRecentSample = (mostRecentSample + 1) % buffer.size();
    buffer[mostRecentSample] = sample;
    if (numSamples < buffer.size())
        numSamples++;
}

void CircularBuffer::addSamples(const float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many samples");
    for (size_t i = 0;i < length;i++)
    {
        mostRecentSample = (mostRecentSample + 1) % buffer.size();
        buffer[mostRecentSample] = samples[i];
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
        mostRecentSample = (mostRecentSample + 1) % buffer.size();
        buffer[mostRecentSample] = samples[i] * gain;
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
    if (mostRecentSample >= delay)
        return buffer[mostRecentSample - delay];
    else
        return buffer[buffer.size() + mostRecentSample - delay];
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
        if (mostRecentSample >= curDelay)
            output[i] = buffer[mostRecentSample - curDelay];
        else
            output[i] = buffer[buffer.size() + mostRecentSample - curDelay];
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
        if (mostRecentSample >= curDelay)
        {
            output[i] += buffer[mostRecentSample - curDelay] * gain;
        }
        else
        {
            float s = buffer[buffer.size() + mostRecentSample - curDelay];
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
        if (mostRecentSample >= curDelay)
        {
            output[i] += buffer[mostRecentSample - curDelay] * gain;
        }
        else
        {
            float s = buffer[buffer.size() + mostRecentSample - curDelay];
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
    size_t index = mostRecentSample - delay;
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
    size_t index = mostRecentSample - delay;
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

// func is a little less elegant that others but it's more efficient this way
void CircularBuffer::applyFilterToSamples
(size_t delay, size_t len, Filter* filter)
{
    TRACE_EVENT_BEGIN("dsp", "filter");
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    // get bounds of area to process
    size_t start = mostRecentSample - delay;
    size_t end = start + len;
    // handle underflow in bounds
    if (start > buffer.size())
        start += buffer.size();
    if (end > buffer.size())
        end += buffer.size();
    if (start > end && end != 0)
    {
        filter->processSamples(buffer.data() + start, buffer.size() - start);
        filter->processSamples(buffer.data(), end);
    }
    else
    {
        filter->processSamples(buffer.data() + start, len);
    }
    TRACE_EVENT_END("dsp");
}

// === Other Operations =======================================================
void CircularBuffer::clear()
{
    mostRecentSample = 0;
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