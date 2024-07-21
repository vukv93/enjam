/** @file */
#pragma once
#include <vector>
#include <optional>
#include "../meg/meg.h"
#include "../libgarage/jack.h"
#include "../libgarage/memory.h"
/** Modular audio engine, the namespace. */
namespace Mae {
  template<class T> using Maybe = std::optional<T>;
  /** MIDI note number to frequrency [Hz] conversion. */
  double midiCps(double midiNote);
  /** The inverse of midiCps. */
  double cpsMidi(double midiNote);
  /** For counting things. */
  using Count = unsigned long;
  /** Base data type. */
  /* @todo[240721_041432] Make NRT_ONLY compile option. */
#ifndef NRT_ONLY
  using Sample = Audio::Jack::Sample;
#else
  using Sample = float;
#endif
  /** Audio data carrier. */
  struct Cable { Sample* m_buffer = nullptr; };
  /** External audio interface. */
  struct ExternCable : Cable {
    Audio::Jack::AudioPort m_port;
  };
  /** Base module interface. */
  struct Module {
    Memory::SimplePool* m_memoryPool;
    std::vector<Cable*> m_inputs;
    std::vector<Cable> m_outputs;
    double m_fs;
    Module(Memory::SimplePool* pool, Count nOut = 1, Count nIn = 1);
    bool HasInput(Count n);
    virtual ~Module();
    virtual int Allocate(Count nFrames);
    virtual int Free();
    virtual int Process(Count nFrames) = 0;
    virtual void SetSampleRate(double fs);
    virtual Count MemoryNeeded(Count nFrames);
  };
  /** Base module container interface. */
  struct Container : Module {
    std::vector<Module*> m_plugins;
    Container(
        Memory::SimplePool* pool,
        std::vector<Module*>&& plugins,
        Count nOut = 1,
        Count nIn = 1);
    virtual int Allocate(Count nFrames);
    virtual int Free();
    virtual void SetSampleRate(double fs);
    virtual Count MemoryNeeded(Count nFrames);
  };
  /** Modular audio engine, the type. */
  struct Mae { Module* m_topLevel; double m_fs; };
  /** Online mode. */
  struct MaeRt : Mae, Audio::Jack::ICallback {
    std::vector<ExternCable> m_inputs;
    std::vector<ExternCable> m_outputs;
    Audio::Jack::Client m_client;
    MaeRt(Module* topLevel);
    void ConnectDefaultOutputs();
    virtual int CallbackProcess(uint32_t nFrames);
    virtual int CallbackSampleRate(uint32_t nFrames);
    virtual int CallbackShutdown();
  };
  /* @todo[240629_071943] Add non-realtime rendering. */
  /** Offline mode. */
  struct MaeNrt {
    using Score = std::vector<Meg::Gef>;
    Module* m_topLevel;
    MaeNrt(Module* topLevel);
    Maybe<std::vector<Sample>> Render(
        double duration, double fs,
        std::vector<std::vector<Sample>>* in = nullptr,
        Count bufferSize = 1024);
    int RenderFile(const std::string& fileName, double duration, double fs);
  };
}
