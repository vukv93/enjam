#include "modlib.hpp"
#include "mae/macros.hpp"
#include <tuple>
#include <concepts>
using namespace std;
namespace Mae {
  namespace Modlib {
    Error ParallelProcessor::Process(Count nFrames) {
      for (Count ch = 0; ch < m_inputs.size(); ch++)
        if (HAS_INPUT(ch)) ProcessChannel(nFrames, ch);
      return ErrorNone;
    }
    Error ParallelProcessor::AddChannels(Count nChannels) {
      for (Count ch = 0; ch < nChannels; ch++) {
        m_inputs.push_back(nullptr);
        m_outputs.push_back({nullptr});
      }
      m_nChannels += nChannels;
      return ErrorNone;
    }
    Error Amp::Hear(Word msg) {
      m_amp = (double)(msg & 0x7f) / 128;
      return ErrorNone;
    }
    Error Amp::ProcessChannel(Count nFrames, Count ch) {
      for (Count i = 0; i < nFrames; i++)
        SAMPLE_OUT(ch,i) = m_amp * MAYBE_SAMPLE_IN_0(ch,i);
      return ErrorNone;
    }
    Error Thru::ProcessChannel(Count nFrames, Count ch) {
      for (Count i = 0; i < nFrames; i++) 
        SAMPLE_OUT(ch,i) = SAMPLE_IN(ch,i);
      return ErrorNone;
    }
    Error PhasorOsc::Process(Count nFrames) {
      for (Count i = 0; i < nFrames; i++) {
        SAMPLE_OUT(0,i) = Tick();
        MAYBE_SAMPLE_IN_PREV(m_freq,0,i);
        if (m_freq < 0) m_freq = -m_freq;
        m_phase += m_freq / m_fs;
        if (m_phase > 1) m_phase -= 1;
      }
      return ErrorNone;
    }
    Error PhasorOsc::Hear(Word msg) {
      m_freq = midiCps(msg & 0x7f);
      return ErrorNone;
    }
    Sample SinOsc::Tick() { return sin(2 * M_PI * m_phase); }
    Sample SawOsc::Tick() { return (.5 - m_phase) * 2; }
    Sample SqrOsc::Tick() { return (m_phase >= .5) * 2 - 1; }
    Error EnvAR::ProcessChannel(Count nFrames, Count ch) {
      for (unsigned long i = 0; i < nFrames; i++) {
        MAYBE_SAMPLE_IN_PREV(m_trigger,ch,i);
        if (m_trigger)
          m_trigger = false, m_delta = 1/(m_fs*m_attack), m_state = Attack;
        switch (m_state) {
          case Idle: 
            break;
          case Attack:
            if (m_level >= 1)
              m_level = 1, m_delta = -1/(m_fs*m_release), m_state = Release;
            break;
          case Release:
            if (m_level <= 0) m_level = 0, m_delta = 0, m_state = Idle;
            break;
        }
        SAMPLE_OUT(ch,i) = m_level += m_delta;
      }
      return ErrorNone;
    }
    Error EnvAR::Hear(Word msg) {
      (void)msg;
      m_trigger = true;
      return ErrorNone;
    }
    Error EnvASR::ProcessChannel(Count nFrames, Count ch) {
      for (unsigned long i = 0; i < nFrames; i++) {
        MAYBE_SAMPLE_IN_PREV(m_gate,ch,i);
        switch (m_state) {
          case Idle: 
            if (m_gate) m_delta = 1/(m_fs*m_attack), m_state = Attack;
            break;
          case Attack:
            if (m_level >= 1) m_level = 1, m_delta = 0, m_state = Sustain;
            break;
          case Sustain:
            if (!m_gate) m_delta = -1/(m_fs*m_release), m_state = Release;
            break;
          case Release:
            if (m_level <= 0) m_level = 0, m_delta = 0, m_state = Idle;
            break;
        }
        SAMPLE_OUT(ch,i) = m_level += m_delta;
      }
      return ErrorNone;
    }
    Error EnvASR::Hear(Word msg) { m_gate = !!(0x7f & msg); return ErrorNone; }
    Error Mul::Process(Count nFrames) {
      for (Count i = 0; i < nFrames; i++) {
        SAMPLE_OUT(0,i) = 1.0;
        for (Count inidx = 0; inidx < m_inputs.size(); inidx++)
          SAMPLE_OUT(0,i) *= MAYBE_SAMPLE_IN_ALT(inidx,i,1.0);
      }
      return ErrorNone;
    }
    Error Add::Process(Count nFrames) {
      for (Count i = 0; i < nFrames; i++) {
        SAMPLE_OUT(0,i) = 0.0;
        for (Count inidx = 0; inidx < m_inputs.size(); inidx++)
          SAMPLE_OUT(0,i) += MAYBE_SAMPLE_IN_ALT(inidx,i,0.0);
      }
      return ErrorNone;
    }
    Error Pan::Process(Count nFrames) {
      for (Count i = 0; i < nFrames; i++) {
        MAYBE_SAMPLE_IN_PREV(m_pan,1,i);
        if (m_pan < -1) m_pan = -1;
        if (m_pan > 1) m_pan = 1;
        Sample in = SAMPLE_IN(0,i);
        SAMPLE_OUT(0,i) = in * (.5-m_pan/2) * m_amp;
        SAMPLE_OUT(1,i) = in * (.5+m_pan/2) * m_amp;
      }
      return ErrorNone;
    }
    Error Pan::Hear(Word msg) {
      m_pan = (double)(msg & 0x7f) / 128 - .5;
      return ErrorNone;
    }
    Error Lag::AddChannels(Count nChannels) {
      /* @todo[240919_095207] Allocation failure handling. */
      for (Count ch = 0; ch < nChannels; ch++) 
        m_dls.push_back(vector<Sample>(m_maxLag,0));
      ParallelProcessor::AddChannels(nChannels);
      return ErrorNone;
    }
    void Lag::SetSampleRate(double fs) {
      m_fs = fs;
      for (auto& dl : m_dls) dl.resize(Count(m_fs * m_maxLag), 0);
    };
    Error Lag::ProcessChannel(Count nFrames, Count ch) {
      /* Average input port samples. */
      Count sampleLag = MIN(Count(m_lag * m_fs), m_dls[ch].size());
      for (Count i = 0; i < nFrames; i++) {
        m_avg -= m_dls[ch][m_offset] / sampleLag;
        m_avg += SAMPLE_IN(ch,i) / sampleLag;
        m_dls[ch][m_offset++] = SAMPLE_IN(ch,i);
        m_offset %= sampleLag;
        SAMPLE_OUT(ch,i) = m_avg;
      }
      return ErrorNone;
    }
    void Lag::SetMaxLag(double maxLag) {
      m_maxLag = maxLag;
      /* @todo[240918_065542] Is this safe while playing? */
      for (auto& dl : m_dls) dl.resize(Count(m_fs * m_maxLag), 0);
    }
    Error Lag::Hear(Word msg) {
      m_lag = m_maxLag * (double)(msg & 0x7f) / 128;
      return ErrorNone;
    }
    Error SoftClip::ProcessChannel(Count nFrames, Count ch) {
      for (Count i = 0; i < nFrames; i++) {
        Sample x = m_inputs[ch]->m_buffer[i] * m_gain, y;
        if (x < -3) y = -1;
        else if (x > 3) y = 1;
        else y = x * (27 + x * x) / (27 + 9 * x * x);
        SAMPLE_OUT(ch,i) = y;
      }
      return ErrorNone;
    }
    Error SoftClip::Hear(Word msg) {
      m_gain = (double)(msg & 0x7f) / 128;
      return ErrorNone;
    }
    void CombBiquad::Reset() {
      for (Count ch = 0; ch < m_inputs.size(); ch++) {
        for (auto& z : m_cdl[ch]) z = 0;
        for (auto& z : m_bqdl[ch]) z = 0;
      }
      m_combPtr = 0;
    }
    Error CombBiquad::ProcessChannel(Count nFrames, Count ch) {
      for (Count i = 0; i < nFrames; i++) {
        Complex x = MAYBE_SAMPLE_IN_0(ch,i);
        Complex co = x - c1 * m_cdl[ch][m_combPtr];
        Complex bn = a0 * (co + a1 * m_bqdl[ch][0] + a2 * m_bqdl[ch][1]);
        Complex y = (b0 * bn + b1 * m_bqdl[ch][0] + b2 * m_bqdl[ch][1]);
        m_cdl[ch][m_combPtr++] = x; if (m_combPtr == m_cdl.size()) m_combPtr = 0;
        m_bqdl[ch][1] = m_bqdl[ch][0]; m_bqdl[ch][0] = bn;
        SAMPLE_OUT(ch,i) = y.real();
      }
      return ErrorNone;
    }
    /* @todo[240919_101608] Test adding channels. */
    Error CombBiquad::AddChannels(Count nChannels) {
      /* @todo[240919_095207] Allocation failure handling. */
      for (Count ch = 0; ch < nChannels; ch++) {
        m_cdl.push_back(vector<Complex>(m_cdl[0].size(),0));
        m_bqdl.push_back(vector<Complex>(2,0));
      }
      ParallelProcessor::AddChannels(nChannels);
      return ErrorNone;
    }
    /* @todo[240831_064622] Can this look a little nicer? */
    ModuleFactory::ModuleFactory() {
      BUILDER_DEFAULT(SinOsc);
      BUILDER_DEFAULT(SawOsc);
      BUILDER_DEFAULT(SqrOsc);
      BUILDER_DEFAULT(EnvAR);
      BUILDER_DEFAULT(EnvASR);
      BUILDER_DEFAULT(Mul);
      BUILDER_DEFAULT(Add);
      BUILDER_DEFAULT(Pan);
      BUILDER_DEFAULT(Lag);
      BUILDER_DEFAULT(SoftClip);
      BUILDER_DEFAULT(Thru);
      BUILDER_DEFAULT(Amp);
    }
  }
};
