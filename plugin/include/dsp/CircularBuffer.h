#pragma once
#include <vector>

class Filter;

class CircularBuffer
{
public:
    // Lifecycle
    CircularBuffer();
    CircularBuffer(size_t capacity);

    // Add Samples
    void addSample(const float sample);
    void addSamples(const float* samples, size_t numToAdd);
    void addSamplesRamped(const float* samples, size_t numToAdd);

    // Get Samples
    float getSample(size_t delay);
    void getSamples(size_t delay, float* output, size_t length);
    void sumWithSamples(size_t delay, float* output, size_t length,
        float gain = 1.0f);
    void sumWithSamplesRamped(size_t delay, float* output, size_t length,
        float startGain = 0, float endGain = 1);

    // Manipulate Samples
    void applyGainToSamples(size_t delay, size_t length, float gain);
    void applyGainToSamples(size_t delay, size_t length, float startGain,
        float endGain);
    void applyFilterToSamples(size_t delay, size_t length, Filter* filter);

    // Other Operations
    void clear();
    void resize(size_t newLength);
    void resize(double sampleRate, float maxDelaySeconds);
    
private:
    std::vector<float> buffer;
    size_t sampleCount;

    size_t capacityMask(size_t sample);
};