#pragma once
#include <vector>

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

    // === Other Operations ===================================================
    void clear();
    void resize(size_t newLength);
    void resize(double sampleRate, float maxDelaySeconds);
    
private:
    std::vector<float> buffer;
    size_t leastRecentSample;
    size_t numSamples;
};