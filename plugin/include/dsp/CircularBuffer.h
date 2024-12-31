#pragma once
#include <vector>

// forward declarations
class Filter;

// if any function calls are given length or delay arguments that are larger
// than the capacity of the buffer, they may result in undefined behavior
class CircularBuffer
{
public:
    // === Lifecycle ==========================================================
    CircularBuffer();
    CircularBuffer(size_t capacity);

    // === Add Samples ========================================================
    void addSample(const float sample);
    void addSamples(const float* samples, size_t length);
    void addSamplesRamped(const float* samples, size_t length);

    // === Get Samples ========================================================
    float getSample(size_t delay);
    void getSamples(size_t delay, float* output, size_t length);
    void sumWithSamples
    (size_t delay, float* output, size_t len, float gain = 1.0f);
    void sumWithSamplesRamped
    (size_t delay, float* output, size_t len, float start = 0, float end = 1);

    // === Manipulate Samples =================================================
    void applyGainToSamples(size_t delay, size_t len, float gain);
    void applyGainToSamples(size_t delay, size_t len, float start, float end);
    void applyFilterToSamples(size_t delay, size_t len, Filter* filter);

    // === Other Operations ===================================================
    void clear();
    void resize(size_t newLength);
    void resize(double sampleRate, float maxDelaySeconds);
    
private:
    std::vector<float> buffer;
    size_t leastRecentSample;
    size_t numSamples;
};