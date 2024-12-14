#include "CircularBuffer.h"
#include <algorithm>
#include <stdexcept>

const size_t CircularBuffer::resamplingSize = 1024;

// === Lifecycle ==============================================================
CircularBuffer::CircularBuffer() : CircularBuffer(44100) { }

CircularBuffer::CircularBuffer(size_t capacity)
    : buffer(capacity), leastRecentSample(0),
    resampleBuffer(resamplingSize, 0)
{ 
    if (capacity == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
    clear();
}

// === Manipulate Samples =====================================================
void CircularBuffer::addSample(const float sample)
{
    mutex.lock();
    leastRecentSample = (leastRecentSample + 1) % buffer.size();
    buffer[leastRecentSample] = sample;
    mutex.unlock();
}

void CircularBuffer::addSamples(const float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many samples");
    mutex.lock();
    for (size_t i = 0;i < length;i++)
    {
        leastRecentSample = (leastRecentSample + 1) % buffer.size();
        buffer[leastRecentSample] = samples[i];
    }
    mutex.unlock();
}

float CircularBuffer::getSampleDelayed(size_t delay)
{
    if (delay > buffer.size())
        throw std::invalid_argument("Delay value too long");
    mutex.lock();
    float r;
    if (leastRecentSample >= delay)
        r = buffer[leastRecentSample - delay];
    else
        r = buffer[buffer.size() + leastRecentSample - delay];
    mutex.unlock();
    return r;
}

void CircularBuffer::getSamplesDelayed
(size_t delay, float* output, size_t len)
{
    if (len > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + len > buffer.size())
        throw std::invalid_argument("Delay value too long");
    mutex.lock();
    for (size_t i = 0;i < len;i++)
    {
        size_t curDelay = delay + len - 1 - i;
        if (leastRecentSample >= curDelay)
            output[i] = buffer[leastRecentSample - curDelay];
        else
            output[i] = buffer[buffer.size() + leastRecentSample - curDelay];
    }
    mutex.unlock();
}

void CircularBuffer::sumWithSamplesDelayed
(size_t delay, float* samples, size_t length)
{
    if (length > buffer.size())
        throw std::invalid_argument("Too many delayed samples requested");
    if (delay + length > buffer.size())
        throw std::invalid_argument("Delay value too long");
    mutex.lock();
    for (size_t i = 0;i < length;i++)
    {
        size_t curDelay = delay + length - 1 - i;
        if (leastRecentSample >= curDelay)
            samples[i] += buffer[leastRecentSample - curDelay];
        else
            samples[i] += buffer[buffer.size() + leastRecentSample - curDelay];
    }
    mutex.unlock();
}

// === Other Operations =======================================================
void CircularBuffer::clear()
{
    mutex.lock();
    std::fill(buffer.begin(), buffer.end(), 0);
    mutex.unlock();
}

void CircularBuffer::resize(size_t newLength)
{
    resizePrivate(newLength);
}

void CircularBuffer::resize(double sampleRate, float maxDelaySeconds)
{
    double maxSamples = sampleRate * maxDelaySeconds;
    size_t newLength = (size_t) std::ceil(maxSamples);
    resizePrivate(newLength);
}

void CircularBuffer::resample(size_t lengthPre, size_t lengthPost)
{
    if (lengthPre > buffer.size() || lengthPost > buffer.size())
        throw std::invalid_argument("Resample length greater than capacity");
    if (lengthPost == lengthPre)
        return;
    mutex.lock();
    // prepare for the resampling
    shiftDataToBufferEdge();
    resampler.reset();
    // calculate resampling parameters
    double ratio = (double) lengthPost / (double) lengthPre;
    const size_t outputBlockSize = (size_t) std::round(ratio * resamplingSize);
    size_t leftToProcess = lengthPre;
    unsigned int blocksProcessed = 0;
    // process whole blocks
    while (leftToProcess >= resamplingSize)
    {
        // copy input samples to the resampling buffer
        for (size_t i = 0;i < resamplingSize;i++)
            resampleBuffer[i] = buffer[buffer.size() - leftToProcess + i];
        // perform resampling for this block
        double spd = (double) resamplingSize / (double) outputBlockSize;
        const float* inputs = resampleBuffer.data();
        size_t offset = blocksProcessed * outputBlockSize;
        float* outputs = buffer.data() + offset;
        resampler.process(spd, inputs, outputs, (int) outputBlockSize);
        leftToProcess -= resamplingSize;
        blocksProcessed++;
    }
    // process remaining samples smaller than a block
    if (leftToProcess > 0)
    {
        // copy input samples to resamling buffer
        for (size_t i = 0;i < leftToProcess;i++)
            resampleBuffer[i] = buffer[buffer.size() - leftToProcess + i];
        // perform resampling for the reamining samples
        int numOutputs = (int) std::round(ratio * leftToProcess);
        double spd = (double) leftToProcess / (double) numOutputs;
        const float* inputs = resampleBuffer.data();
        size_t offset = blocksProcessed * outputBlockSize;
        float* outputs = buffer.data() + offset;
        resampler.process(spd, inputs, outputs, numOutputs);
    }
    // resampling is done, update buffer position
    leastRecentSample = lengthPost;
    mutex.unlock();
}

// === Other Operations =======================================================
void CircularBuffer::shiftDataToBufferEdge()
{
    // Do nothing if data happens to be at buffer edge already
    if (leastRecentSample == buffer.size() - 1)
        return;
    // move samples through "array rotation" implemented as 3 reversals
    // first, reverse entire array
    for (size_t i = 0;i < buffer.size() / 2;i++)
    {
        float tmp = buffer[i];
        size_t swapIndex = buffer.size() - 1 - i;
        buffer[i] = buffer[swapIndex];
        buffer[swapIndex] = tmp;
    }
    // second, reverse portion from end to (now flipped) least recent sample
    size_t revTwoStart = buffer.size() - leastRecentSample - 1;
    for (size_t i = 0;i < (leastRecentSample + 1) / 2;i++)
    {
        size_t idx1 = i + revTwoStart;
        size_t idx2 = buffer.size() - 1 - i;
        float tmp = buffer[idx1];
        buffer[idx1] = buffer[idx2];
        buffer[idx2] = tmp;
    }
    // third, reverse the complement of the set of samples reversed in step 2
    for (size_t i = 0;i < revTwoStart / 2;i++)
    {
        float tmp = buffer[i];
        size_t swapIndex = revTwoStart - 1 - i;
        buffer[i] = buffer[swapIndex];
        buffer[swapIndex] = tmp;
    }
    leastRecentSample = buffer.size() - 1;
}

void CircularBuffer::resizePrivate(size_t newLength)
{
    if (newLength == 0)
        throw std::invalid_argument("Cannot have CircularBuffer of size 0");
    mutex.lock();
    buffer.resize(newLength);
    leastRecentSample = 0;
    std::fill(buffer.begin(), buffer.end(), 0);
    mutex.unlock();
}