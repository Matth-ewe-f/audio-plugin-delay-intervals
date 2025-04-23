#pragma once
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <melatonin_perfetto/melatonin_perfetto.h>
#include "CircularBuffer.h"
#include "DelayAmp.h"
#include "Filter.h"

typedef struct NoteValue
{
    std::string name;
    float proportion;
}
NoteValue;

class PluginProcessor final : public juce::AudioProcessor
{
public:
    juce::AudioProcessorValueTreeState tree;

    PluginProcessor();
    ~PluginProcessor() override;

    inline const juce::String getName() const override
    {
        return "Delay Intervals";
    }

    inline bool hasEditor() const override
    {
        return true;
    }
    
    inline int getNumPrograms() override
    {
        return 1;
    }
    
    inline int getCurrentProgram() override
    {
        return 0;
    }

    inline void setCurrentProgram(int index) override
    {
        juce::ignoreUnused(index);
    }

    inline const juce::String getProgramName(int index) override
    {
        juce::ignoreUnused(index);
        return {};
    }

    inline void changeProgramName(int i, const juce::String& name) override
    {
        juce::ignoreUnused(i, name);
    }

    inline bool acceptsMidi() const override
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    inline bool producesMidi() const override
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    inline bool isMidiEffect() const override
    {
#if JucePlugin_IsMidiEffect
        return true;
#else
        return false;
#endif
    }
    
    inline std::string getIdForLeftIntervalAmp(int index)
    {
        return "left-delay" + std::to_string(index);
    }

    inline std::string getIdForRightIntervalAmp(int index)
    {
        return "right-delay-" + std::to_string(index);
    }

    double getTailLengthSeconds() const override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    float getSecondsForNoteValue(int index);

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
        override;

    void resetLeftAmps();
    void resetRightAmps();
    void copyLeftAmpsToRight();
    void copyRightAmpsToLeft();

    void linkFilters();
    void unlinkFilters();
    void linkDelays();
    void unlinkDelays();

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void notifyHostOfStateChange();

    juce::AudioProcessorEditor* createEditor() override;

private:
    static constexpr int maxIntervals = 16;
    static const float maxDelayTime;

    static constexpr size_t numNoteValues = 6;
    static const NoteValue noteValues[numNoteValues];
    
    double lastSampleRate;
    double lastBpm;
    bool lastAmpsLinked;
    bool lastFiltersLinked;
    
    size_t numSamples;
    size_t currentDelay;
    size_t lastBlockDelay;
    size_t currentNumIntervals;
    size_t lastBlockNumIntervals;

    float currentWet;
    float lastBlockWet;
    float currentFalloff;
    float lastBlockFalloff;

    bool loop;
    bool lastBlockLoop;
    bool fadeOut;
    bool lastBlockFadeOut;

    CircularBuffer leftBuffer;
    CircularBuffer rightBuffer;
    DelayAmp leftAmps[maxIntervals];
    DelayAmp rightAmps[maxIntervals];
    Filter leftFilters[maxIntervals];
    Filter rightFilters[maxIntervals];

    std::vector<float> tempBuffer; // for operating on signal in processBlock
#if PERFETTO
    std::unique_ptr<perfetto::TracingSession> tracingSession;
#endif

    void handleTempoSync();
    void handleParameterLinking();
    void updateCurrentBlockParameters();
    void updateLastBlockParameters();
    void updateParametersOnReset();

    void processChannel(float* audio, CircularBuffer* buffer, DelayAmp* amps,
        Filter* filters);
    void processDrySignal(float* audio, DelayAmp* amps);
    void processLoopedSignal(float* audio, CircularBuffer* buffer,
        DelayAmp* amps, Filter* filters);
    void pushBlockSamplesOntoBuffer(float* audio, CircularBuffer* buffer);
    void processWetSignal(float* audio, CircularBuffer* buffer, DelayAmp* amps,
        Filter* filters);

    BusesProperties createBusesProperties();
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    size_t getDelaySamples();
    size_t getCurrentNumIntervals();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
