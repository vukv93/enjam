/** @file */
#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <complex>
#include <samplerate.h>
#include "mae.h"
#include "../libgarage/sndfile.h"
/* @todo[240718_035701] Make all existing modules work with uninitialized
 * inputs. (HasInput(idx) ? m_inputs[idx]->buffer[i] : 0) */
#define MAYBE_INPUT(inIdx,i) (HasInput(inIdx) ? m_inputs[inIdx]->m_buffer[i] : 0.0)
namespace Mae {
  using Sample = Audio::Jack::Sample;
  using Complex = std::complex<Sample>;
  /** Mock module. */
  struct TestModule : Module {
    TestModule(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Passthrough. */
  struct Thru : Module {
    Thru(Memory::SimplePool* pool, Count n);
    int Process(Count nFrames);
  };
  /** Swiss Army knife */
  struct CombBiquad : Module {
    std::vector<Complex> m_cdl;
    std::vector<Complex> m_bqdl;
    Complex a0,a1,a2,b0,b1,b2,c1;
    Count m_combPtr = 0;
    CombBiquad(Memory::SimplePool* pool, Count combLength);
    void Reset();
    int Process(Count nFrames);
  };
  /** Soft clipping. */
  struct SoftClip : Module {
    double m_amp = 0.1, m_gain = 3;
    SoftClip(Memory::SimplePool* pool, Count n);
    int Process(Count nFrames);
  };
  /** Sine oscillator. */
  struct SinOsc : Module { /* Sample rate must propagate. */
    double m_phase = 0, m_freq = 440, m_amp = 0.1;
    SinOsc(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Saw oscillator. */
  struct SawOsc : Module {
    double m_phase = 0, m_freq = 440, m_amp = 0.1;
    SawOsc(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Square oscillator. */
  struct SqrOsc : Module {
    double m_phase = 0, m_freq = 440, m_amp = 0.1;
    SqrOsc(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Attack-release envelope generator. */
  struct EnvAR : Module {
    enum State {Idle, Attack, Release};
    bool m_trigger = false;
    double m_attack = 0.1, m_release = 0.1;
    double m_level = 0, m_delta = 0;
    State m_state = Idle;
    EnvAR(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  struct EnvASR : Module {
    enum State {Idle, Attack, Sustain, Release};
    bool m_gate = false;
    double m_attack = .1, m_release = .1;
    double m_level = 0, m_delta = 0;
    State m_state = Idle;
    EnvASR(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Controlled amplifier, envelope applier. */
  struct VCA : Module {
    VCA(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
  /** Mixer (addition) module. */
  /* @todo[240704_060850] Add level controls. */
  struct Mixer : Module {
    Mixer(Memory::SimplePool* pool, Count nInputs);
    int Process(Count nFrames);
  };
  /** Panning module. */
  struct Pan : Module {
    Pan(Memory::SimplePool* pool);
    double m_pan = 0;
    int Process(Count nFrames);
  };
  struct Lag : Module {
    double m_lag = 0.2;
    double m_currOut = 0, m_in = 0, m_prevIn = 0;
    double m_delta = 0;
    Lag(Memory::SimplePool* pool, double initValue = 0);
    int Process(Count nFrames);
  };
  struct SimpleSampler : Module, Audio::InFile {
    std::vector<Sample> m_sample;
    Count m_phase = 0;
    bool m_loop = false;
    bool m_running = true;
    SimpleSampler(Memory::SimplePool* pool, const std::string& fileName);
    void SetSampleRate(double fs);
    virtual int Process(Count nFrames);
  };
  struct MultiTriggerSampler : SimpleSampler {
    struct Phase { Count phase; bool active; };
    std::vector<Phase> m_phases;
    bool m_trigger = false;
    MultiTriggerSampler(
        Memory::SimplePool* pool,
        const std::string& fileName,
        Count nPhases);
    int Process(Count nFrames);
  };
  struct Resampler : Module {
    /* @todo[240701_171605] Real-time resampling module with libsamplerate. */
    Resampler(Memory::SimplePool* pool);
    int Process(Count nFrames);
  };
}
