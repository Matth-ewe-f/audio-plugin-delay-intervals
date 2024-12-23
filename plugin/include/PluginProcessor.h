#pragma once
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <melatonin_perfetto/melatonin_perfetto.h>
#include "CircularBuffer.h"
#include "DelayAmp.h"
#include "DelayInterval.h"

class PluginProcessor final : public juce::AudioProcessor
{
public:
    // === Public Variables ===================================================
    juce::AudioProcessorValueTreeState tree;

    // === Lifecycle ==========================================================
    PluginProcessor();
    ~PluginProcessor() override;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // === Plugin Information =================================================
    inline const juce::String getName() const override
        { return "Delay-Intervals"; }

    inline bool hasEditor() const override { return true; }
    inline double getTailLengthSeconds() const override { return 1.0; }
    
    inline int getNumPrograms() override { return 1; } // should always be >= 1
    inline int getCurrentProgram() override { return 0; }
    inline void setCurrentProgram (int index) override
        { juce::ignoreUnused(index); }
    inline const juce::String getProgramName (int index) override
        { juce::ignoreUnused(index); return {}; }
    inline void changeProgramName (int i, const juce::String& name) override
        { juce::ignoreUnused(i, name); }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // === Midi Information ===================================================
    inline bool acceptsMidi() const override
#if JucePlugin_WantsMidiInput
        { return true; }
#else
        { return false; }
#endif

    inline bool producesMidi() const override
#if JucePlugin_ProducesMidiOutput
        { return true; }
#else
        { return false; }
#endif

    inline bool isMidiEffect() const override
#if JucePlugin_IsMidiEffect
        { return true; }
#else
        { return false; }
#endif

    // === Processing Audio ===================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // === State ==============================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // === Factory Functions ==================================================
    juce::AudioProcessorEditor* createEditor() override;
    
    // === Parameter Information ==============================================
    inline std::string getIdForLeftIntervalAmp(int index)
        { return "left-delay" + std::to_string(index); }
    inline std::string getIdForRightIntervalAmp(int index)
        { return "right-delay-" + std::to_string(index); }

private:
    double lastSampleRate;
    size_t lastDelay;
    size_t lastIntervals;
    float lastDryWet;
    float lastFalloff;
    bool lastLoop;
    bool lastBlockFadeOut;
    DelayAmp leftAmps[16];
    CircularBuffer leftBuffer;
    Filter leftFilters[16];
    DelayAmp rightAmps[16];
    CircularBuffer rightBuffer;
    Filter rightFilters[16];
    std::vector<float> tempBuffer; // for operating on signal in processBlock
#if PERFETTO
    std::unique_ptr<perfetto::TracingSession> tracingSession;
#endif

    // === Constants ==========================================================
    static const float maxDelayTime;
    static const int maxIntervals;

    // === Private Helper =====================================================
    size_t getDelaySamples();
    size_t getCurrentNumIntervals();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
