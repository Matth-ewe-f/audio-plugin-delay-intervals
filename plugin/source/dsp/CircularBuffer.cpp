#include "CircularBuffer.h"
#include <algorithm>
#include <stdexcept>

// === Lifecycle ==============================================================
CircularBuffer::CircularBuffer() : CircularBuffer(44100) { }

CircularBuffer::CircularBuffer(size_t capacity)
    : buffer(capacity), leastRecentSample(0)
{ 
    if (capacity == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
    clear();
}

// === Manipulate Samples =====================================================
void CircularBuffer::addSample(const float sample)
{
    leastRecentSample = (leastRecentSample + 1) % buffer.size();
    buffer[leastRecentSample] = sample;
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
}

float CircularBuffer::getSampleDelayed(size_t delay)
{
    if (delay > buffer.size())
        throw std::invalid_argument("Delay value too long");
    if (leastRecentSample >= delay)
        return buffer[leastRecentSample - delay];
    else
        return buffer[buffer.size() + leastRecentSample - delay];
}

void CircularBuffer::getSamplesDelayed
(size_t delay, float* output, size_t len)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    for (size_t i = 0;i < len;i++)
    {
        size_t curDelay = delay + len - 1 - i;
        if (leastRecentSample >= curDelay)
            output[i] = buffer[leastRecentSample - curDelay];
        else
            output[i] = buffer[buffer.size() + leastRecentSample - curDelay];
    }
}

void CircularBuffer::sumWithSamplesDelayed
(size_t delay, float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + length > buffer.size())
        throw std::invalid_argument("Delay value too long");
    for (size_t i = 0;i < length;i++)
    {
        size_t curDelay = delay + length - 1 - i;
        if (leastRecentSample >= curDelay)
            samples[i] += buffer[leastRecentSample - curDelay];
        else
            samples[i] += buffer[buffer.size() + leastRecentSample - curDelay];
    }
}

// === Other Operations =======================================================
void CircularBuffer::clear()
{
    std::fill(buffer.begin(), buffer.end(), 0);
}

void CircularBuffer::resize(size_t newLength)
{
    if (newLength == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
    buffer.resize(newLength);
    leastRecentSample = 0;
    clear();
}

void CircularBuffer::resize(double sampleRate, float maxDelaySeconds)
{
    double maxSamples = sampleRate * maxDelaySeconds;
    size_t newLength = (size_t) std::ceil(maxSamples);
    resize(newLength);
}