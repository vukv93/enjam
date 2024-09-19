/** @file */
#pragma once
#include <vector>
#include <string>
#include <optional>
#include <map>
#include "libgarage/memory/memory.hpp"
/** Modular audio engine, the namespace. */
namespace Mae {
  /** Default audio sample type. */
  using Sample = double;
  /* @todo[240904_162608] Use an abstraction. */
  /** Memory pool. */
  using Pool = Memory::Pool;
  /** Audio data carrier. */
  struct Cable { Sample* m_buffer = nullptr; };
  /** Is it really there? */
  template<class T> using Maybe = std::optional<T>;
  /** Error codes. */
  enum Error {
    ErrorNone = 0,
    ErrorSome,
  };
  /** Simple module message. */
  using Word = uint32_t;
  /** Basic counting type. */
  using Count = unsigned long;
  /** Base module interface. */
  struct Module {
    struct Arg { void* pool; Count nOutputs; Count nInputs; };
    using MessageHandler = Error(Module*,void*);
    std::vector<Cable*> m_inputs;
    std::vector<Cable> m_outputs;
    double m_fs;
    MessageHandler* m_receiver = nullptr;
    Module();
    Module(Arg* arg);
    Module(const Arg& arg);
    virtual ~Module(){}
    /** Front door for dynamic extension. */
    Error Receive(void* msg);
    /** Digest a 32-bit message. */
    virtual Error Hear(Word msg);
    /** Allocate output buffers. */
    virtual Error Allocate(Pool* pool, Count nFrames);
    /** Process sample block. */
    virtual Error Process(Count nFrames) = 0;
    /** Set the sample rate, propagate. */
    virtual void SetSampleRate(double fs);
  };
  /** Abstract module factory. */
  struct Factory {
    using Builder = Module*();
    std::vector<Factory*> m_delegates{};
    std::map<std::string, Builder*> m_builders{};
    virtual ~Factory() {}
    /** Build a module, with delegation. */
    virtual Module* Build(const std::string& type);
  };
  /** Default module builder function template. */
  template<class T> Module* DefaultBuilder() { return new T(); }
  /** Base module container interface. */
  struct Container : Module {
    Pool* m_memoryPool = nullptr;
    Factory* m_factory = nullptr;
    Container() {}
    Container(Arg* arg) : Module(arg) {}
    Container(const Arg& arg) : Module(arg) {}
  };
  /** Modular audio engine, the type. */
  struct Mae {
    Module* m_topLevel; /**< Top-level module. */
    double m_fs = 0;    /**< Sample rate. */
  };
  /** MIDI note number to frequrency [Hz] conversion. */
  Sample midiCps(Sample midiNote);
  /** The inverse of midiCps. */
  Sample cpsMidi(Sample midiNote);
}
