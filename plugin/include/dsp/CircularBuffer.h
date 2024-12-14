#pragma once
#include <vector>
#include <mutex>
#include <juce_audio_processors/juce_audio_processors.h>

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
    void sumWithSamplesDelayed(size_t delay, float* samples, size_t length);

    // === Other Operations ===================================================
    void clear();
    void resize(size_t newLength);
    void resize(double sampleRate, float maxDelaySeconds);
    void resample(size_t lengthPre, size_t lengthPost);

private:
    std::vector<float> buffer;
    size_t leastRecentSample;
    juce::WindowedSincInterpolator resampler;
    std::vector<float> resampleBuffer;
    std::mutex mutex;

    static const size_t resamplingSize;

    // === Private Helper =====================================================
    void shiftDataToBufferEdge();
    void resizePrivate(size_t newLength);
};