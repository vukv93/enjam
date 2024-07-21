/** @file */
#pragma once
#include <vector>
#include "../libgarage/jack.h"
#include "../libgarage/memory.h"
/** Modular audio engine. */
namespace Mae {
  /** MIDI note number to frequrency [Hz] conversion. */
  double midiCps(double midiNote);
  /** The inverse of midiCps. */
  double cpsMidi(double midiNote);
  /** For counting things. */
  using Count = unsigned long;
  /** Base data type. */
  using Sample = Audio::Jack::Sample;
  /** Audio data carrier. */
  struct Cable { Sample* m_buffer = nullptr; };
  /** External audio interface. */
  struct ExternCable : Cable { Audio::Jack::AudioPort m_port; };
  /** Base module interface. */
  struct Module {
    Memory::SimplePool* m_memoryPool;
    std::vector<Cable*> m_inputs;
    std::vector<Cable> m_outputs;
    double m_fs;
    Module(Memory::SimplePool* pool, Count nOut = 1, Count nIn = 1);
    bool HasInput(Count n);
    virtual ~Module();
    virtual void SetSampleRate(double fs);
    virtual int Allocate(Count nFrames);
    virtual int Process(Count nFrames) = 0;
  };
  /** Base module container interface. */
  struct Container : Module {
    std::vector<Module*> m_plugins;
    Container(
        Memory::SimplePool* pool,
        std::vector<Module*>&& plugins,
        Count nOut = 1,
        Count nIn = 1);
    virtual void SetSampleRate(double fs);
  };
  /** The system. */
  struct Mae : Audio::Jack::ICallback {
    Module* m_topLevel;
    Audio::Jack::Client m_client;
    std::vector<ExternCable> m_inputs;
    std::vector<ExternCable> m_outputs;
    Sample m_fs;
    Mae(Module* topLevel);
    void ConnectDefaultOutputs();
    virtual int CallbackProcess(uint32_t nFrames);
    virtual int CallbackSampleRate(uint32_t nFrames);
    virtual int CallbackShutdown();
  };
  /* @todo[240629_071943] Add non-realtime rendering. */
  struct MaeNrt {
    Module* m_topLevel;
    MaeNrt(Module* topLevel);
    Sample* Render(double duration, double fs);
    void RenderFile(const std::string& fileName, double duration, double fs);
  };
}
