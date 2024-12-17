#pragma once
#include <vector>

class CircularBuffer
{
public:
    // === Lifecycle ==========================================================
    CircularBuffer();
    CircularBuffer(size_t capacity);

    // === Manipulate Samples =================================================
    void addSample(const float sample);
    void addSamples(const float* samples, size_t length);
    float getSampleDelayed(size_t delay);
    void getSamplesDelayed(size_t delay, float* output, size_t length);

    // === Other Operations ===================================================
    void clear();
    void resize(size_t newLength);
    void resize(double sampleRate, float maxDelaySeconds);
private:
    std::vector<float> buffer;
    size_t leastRecentSample;
    size_t numSamples;
};