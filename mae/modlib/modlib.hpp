/** @file */
#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <complex>
#include <functional>
#include "mae/mae.hpp"
namespace Mae {
  /** A collection of simple modules. */
  namespace Modlib {
    using Complex = std::complex<Sample>;
    /** Multiple channel parallel processor abstraction. */
    struct ParallelProcessor : Module {
      Count m_nChannels = 1;
      ParallelProcessor() : Module({nullptr,1,1}) {}
      Error Process(Count nFrames);
      virtual Error AddChannels(Count nChannels = 1);
      virtual Error ProcessChannel(Count nFrames, Count ch) = 0;
    };
    /** Simple amplifier. */
    struct Amp : ParallelProcessor {
      double m_amp = 1;
      Error ProcessChannel(Count nFrames, Count channel);
      Error Hear(Word msg);
    };
    /** Passthrough. */
    struct Thru : ParallelProcessor {
      Error ProcessChannel(Count nFrames, Count ch);
    };
    /** Abstract mono generator. */
    struct Amg : Module {
      Amg() : Module({nullptr,1,0}) {}
    };
    /** Abstract continuous-phase oscillator. */
    struct PhasorOsc : Amg {
      double m_phase = 0, m_freq = 440;
      Error Process(Count nFrames);
      Error Hear(Word msg);
      virtual Sample Tick() = 0;
    };
    /** Phasor oscillator template. */
    /** Sine oscillator. */
    struct SinOsc : PhasorOsc { Sample Tick(); };
    /** Saw oscillator. */
    struct SawOsc : PhasorOsc { Sample Tick(); };
    /** Square oscillator. */
    struct SqrOsc : PhasorOsc { Sample Tick(); };
    /* @todo[240909_200053] Envelope generator abstraction? */
    /** Attack-release envelope generator. */
    struct EnvAR : ParallelProcessor {
      enum State {Idle, Attack, Release};
      bool m_trigger = false;
      double m_attack = 0.1, m_release = 0.1;
      double m_level = 0, m_delta = 0;
      State m_state = Idle;
      Error ProcessChannel(Count nFrames, Count ch);
      Error Hear(Word msg);
    };
    /** Attack-sustain-release envelope generator. */
    struct EnvASR : ParallelProcessor {
      enum State {Idle, Attack, Sustain, Release};
      bool m_gate = false;
      double m_attack = .1, m_release = .1;
      double m_level = 0, m_delta = 0;
      State m_state = Idle;
      Error ProcessChannel(Count nFrames, Count ch);
      Error Hear(Word msg);
    };
    /** Controlled amplifier, envelope applier. */
    struct Mul : Module {
      Mul() : Module({nullptr,1,2}) {}
      Error Process(Count nFrames);
    };
    /** Mix (addition) module. */
    struct Add : Module {
      Add() : Module({nullptr,1,2}) {}
      Error Process(Count nFrames);
    };
    /** Panning module. */
    struct Pan : Module {
      double m_pan = 0, m_amp = 1;
      Pan() : Module({nullptr,2,1}) {}
      Error Process(Count nFrames);
      Error Hear(Word msg);
    };
    /** Smooth transitions. */
    struct Lag : ParallelProcessor {
      double m_lag = .02;
      double m_maxLag = .5;
      double m_avg = 0;
      std::vector<std::vector<double>> m_dls;
      Count m_offset = 0;
      Lag() : m_dls(1) {}
      Error ProcessChannel(Count nFrames, Count ch);
      Error AddChannels(Count nChannels);
      /** Also actually allocates the buffer. */
      void SetSampleRate(double fs);
      void SetMaxLag(double maxLag);
      Error Hear(Word msg);
    };
    /** Soft clipping.
     * https://paulbatchelor.github.io/sndkit/softclip/ */
    struct SoftClip : ParallelProcessor {
      double m_gain = 3;
      Error ProcessChannel(Count nFrames, Count ch);
      Error Hear(Word msg);
    };
    /** DSP Swiss Army knife. An example of non-default-constructible module.
     * https://www.dsprelated.com/showarticle/972.php */
    struct CombBiquad : ParallelProcessor {
      std::vector<std::vector<Complex>> m_cdl;
      std::vector<std::vector<Complex>> m_bqdl;
      Complex a0 = 1, a1 = 1, a2 = 0,
              b0 = 1.0/m_cdl.size(), b1 = 0, b2 = 0,
              c1 = 1;
      Count m_combPtr = 0;
      CombBiquad(Count combLength) :
        m_cdl(1, std::vector<Complex>(combLength,0)),
        m_bqdl(1, std::vector<Complex>(2,0)) {}
      void Reset();
      Error SetCombLength(Count n);
      Error ProcessChannel(Count nFrames, Count ch);
      Error AddChannels(Count nChannels);
    };
    /* @todo[240816_034929] Implement compander. */
    struct Compander : ParallelProcessor {};
    /** Modlib module factory. */
    struct ModuleFactory : Factory { ModuleFactory(); };
  }
}
