#include "mae-modlib.h"
namespace Mae {
  TestModule::TestModule(Memory::SimplePool* pool) : Module(pool) {}
  int TestModule::Process(Count nFrames) { return 0; }
  Thru::Thru(Memory::SimplePool* pool, Count n) : Module(pool,n,n) {}
  int Thru::Process(Count nFrames) {
    auto currentIn = m_inputs.begin();
    auto currentOut = m_outputs.begin();
    while (currentIn != m_inputs.end() && currentOut != m_outputs.end()) {
      auto out = *currentOut++;
      auto in = *currentIn++;
      for (Count i = 0; i < nFrames; i++) 
        out.m_buffer[i] = in->m_buffer[i];
    }
    return 0;
  }
  CombBiquad::CombBiquad(Memory::SimplePool* pool, Count combLength) :
    Module(pool,1,1),
    m_cdl(combLength,0),
    m_bqdl(2,0)
  {}
  void CombBiquad::Reset() {
    for (auto& z : m_cdl) z = 0;
    for (auto& z : m_bqdl) z = 0;
    m_combPtr = 0;
  }
  int CombBiquad::Process(Count nFrames) {
    for (Count i = 0; i < nFrames; i++) {
      Complex x = m_inputs[0]->m_buffer[i];
      Complex co = x - c1 * m_cdl[m_combPtr];
      Complex bn = a0 * (co + a1 * m_bqdl[0] + a2 * m_bqdl[1]);
      Complex y = (b0 * bn + b1 * m_bqdl[0] + b2 * m_bqdl[1]);
      m_cdl[m_combPtr++] = x; if (m_combPtr == m_cdl.size()) m_combPtr = 0;
      m_bqdl[1] = m_bqdl[0]; m_bqdl[0] = bn;
      m_outputs[0].m_buffer[i] = y.real();
    }
    return 0;
  }
  SoftClip::SoftClip(Memory::SimplePool* pool, Count n) : Module(pool,n,n) {}
  int SoftClip::Process(Count nFrames) {
    auto currentIn = m_inputs.begin();
    auto currentOut = m_outputs.begin();
    while (currentIn != m_inputs.end() && currentOut != m_outputs.end()) {
      auto out = *currentOut++;
      auto in = *currentIn++;
      for (Count i = 0; i < nFrames; i++) 
        /* @todo[240627_191024] Implement clipping. */
        out.m_buffer[i] = in->m_buffer[i];
    }
    return 0;
  }
  SinOsc::SinOsc(Memory::SimplePool* pool) : Module(pool,1,0) {}
  int SinOsc::Process(Count nFrames) {
    for (unsigned long i = 0; i < nFrames; i++) {
      m_outputs[0].m_buffer[i] = m_amp * sin(2 * M_PI * m_phase);
      if (HasInput(0)) m_phase += m_inputs[0]->m_buffer[i] / m_fs;
      else m_phase += m_freq / m_fs;
      if (m_phase > 1) m_phase -= 1;
    }
    return 0;
  }
  SawOsc::SawOsc(Memory::SimplePool* pool) : Module(pool,1,0) {}
  int SawOsc::Process(Count nFrames) {
    for (unsigned long i = 0; i < nFrames; i++) {
      m_outputs[0].m_buffer[i] = m_amp * (0.5 - m_phase) * 2;
      if (HasInput(0)) m_phase += m_inputs[0]->m_buffer[i] / m_fs;
      else m_phase += m_freq / m_fs;
      if (m_phase >= 1) m_phase = 0;
    }
    return 0;
  }
  SqrOsc::SqrOsc(Memory::SimplePool* pool) : Module(pool,1,0) {}
  int SqrOsc::Process(Count nFrames) {
    for (unsigned long i = 0; i < nFrames; i++) {
      m_outputs[0].m_buffer[i] = m_amp * ((m_phase >= .5)*2-1) * 2;
      if (HasInput(0)) m_phase += m_inputs[0]->m_buffer[i] / m_fs;
      else m_phase += m_freq / m_fs;
      if (m_phase >= 1) m_phase = 0;
    }
    return 0;
  }
  EnvAR::EnvAR(Memory::SimplePool* pool) : Module(pool,1,0) {}
  int EnvAR::Process(Count nFrames) {
    for (unsigned long i = 0; i < nFrames; i++) {
      if (HasInput(0)) m_trigger = m_inputs[0]->m_buffer[i];
      switch (m_state) {
        case Idle: 
          if (m_trigger)
            m_trigger = false, m_delta = 1/(m_fs*m_attack), m_state = Attack;
          break;
        case Attack:
          if (m_level >= 1)
            m_level = 1, m_delta = -1/(m_fs*m_release), m_state = Release;
          break;
        case Release:
          if (m_level <= 0) m_level = 0, m_delta = 0, m_state = Idle;
          break;
      }
      m_outputs[0].m_buffer[i] = m_level += m_delta;
    }
    return 0;
  }
  EnvASR::EnvASR(Memory::SimplePool* pool) : Module(pool,1,0) {}
  int EnvASR::Process(Count nFrames) {
    for (unsigned long i = 0; i < nFrames; i++) {
      if (HasInput(0)) m_gate = m_inputs[0]->m_buffer[i];
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
      m_outputs[0].m_buffer[i] = m_level += m_delta;
    }
    return 0;
  }
  VCA::VCA(Memory::SimplePool* pool) : Module(pool,1,2) {}
  int VCA::Process(Count nFrames) {
    for (Count i = 0; i < nFrames; i++)
      m_outputs[0].m_buffer[i]
        = MAYBE_INPUT(0,i)
        * MAYBE_INPUT(1,i);
    return 0;
  }
  Mixer::Mixer(Memory::SimplePool* pool, Count nInputs) :
    Module(pool,1,nInputs) {}
  int Mixer::Process(Count nFrames) {
    for (Count i = 0; i < nFrames; i++) {
      m_outputs[0].m_buffer[i] = 0.0;
      for (auto& in : m_inputs)
        if (in != nullptr && in->m_buffer != nullptr)
          m_outputs[0].m_buffer[i] += in->m_buffer[i];
    }
    return 0;
  }
  Pan::Pan(Memory::SimplePool* pool) : Module(pool,2,1) {}
  int Pan::Process(Count nFrames) {
    for (Count i = 0; i < nFrames; i++) {
      if (HasInput(1)) m_pan = m_inputs[1]->m_buffer[i];
      if (m_pan < -1) m_pan = -1;
      if (m_pan > 1) m_pan = 1;
      m_outputs[0].m_buffer[i]
        = m_inputs[0]->m_buffer[i] * (.5-m_pan/2);
      m_outputs[1].m_buffer[i]
        = m_inputs[0]->m_buffer[i] * (.5+m_pan/2);
    }
    return 0;
  }
  Lag::Lag(Memory::SimplePool* pool, double initValue) :
    Module(pool,1,0), m_in(initValue), m_prevIn(initValue)
  {}
  int Lag::Process(Count nFrames) {
    if (m_in != m_prevIn) {
      m_delta = (m_in - m_currOut)/(m_fs*m_lag);
      m_prevIn = m_in;
    }
    for (int i = 0; i < nFrames; i++) {
      m_currOut += m_delta;
      if ((m_delta > 0 && m_currOut >= m_in)
          || (m_delta < 0 && m_currOut <= m_in)) m_delta = 0;
      m_outputs[0].m_buffer[i] = m_currOut;
    }
    return 0;
  }
  SimpleSampler::SimpleSampler(Memory::SimplePool* pool, const std::string& fileName) :
    Module(pool,1,0),
    InFile(fileName),
    m_sample(m_sfInfo.frames * m_sfInfo.channels)
  {
    Read(m_sample);
  }
  void SimpleSampler::SetSampleRate(double fs) {
    m_fs = fs;
    if ((int)m_fs != m_sfInfo.samplerate) {
      /* Read file again, resample, store in m_sample. */
      Count originalLength = m_sfInfo.frames * m_sfInfo.channels;
      std::vector<Sample> original(originalLength);
      Reset(); Read(original);
      /* @todo[240702_032209] Think about padding? */
      m_sample.resize((int)(originalLength * m_fs / m_sfInfo.samplerate));
      SRC_DATA srcData{};
      srcData.data_in = original.data();
      srcData.input_frames = m_sfInfo.frames;
      srcData.data_out = m_sample.data();
      srcData.output_frames = m_sample.size() / m_sfInfo.channels;
      srcData.src_ratio = m_fs / m_sfInfo.samplerate;
      src_simple(&srcData, SRC_SINC_BEST_QUALITY, m_sfInfo.channels);
      m_sample.resize(srcData.output_frames_gen * m_sfInfo.channels);
    }
  }
  int SimpleSampler::Process(Count nFrames) {
    if (m_running)
      for (Count i = 0; i < nFrames; i++) {
        /* Plays only first channel. */
        m_outputs[0].m_buffer[i] = m_sample[m_phase * m_sfInfo.channels];
        if (++m_phase >= m_sample.size() / m_sfInfo.channels) {
          /* End of sample. */
          m_phase = 0;
          if (!m_loop) m_running = false;
        }
      }
    else for (Count i = 0; i < nFrames; i++) m_outputs[0].m_buffer[i] = 0;
    return 0;
  }
  MultiTriggerSampler::MultiTriggerSampler(
      Memory::SimplePool* pool,
      const std::string& fileName,
      Count nPhases) :
    SimpleSampler(pool, fileName),
    m_phases(nPhases, {0,false})
    {}
  int MultiTriggerSampler::Process(Count nFrames) {
    /* Activate an idle phase on trigger. */
    if (m_trigger) {
      m_trigger = false;
      for (auto& p : m_phases) { if (!p.active) { p.active = true; break; } }
      /* @todo[240703_021512] What to do when all are active? */
    }
    /* Mix all active phases. */
    for (Count i = 0; i < nFrames; i++) {
      m_outputs[0].m_buffer[i] = 0.0;
      for (auto& p : m_phases)
        if (p.active) {
          m_outputs[0].m_buffer[i] += m_sample[p.phase * m_sfInfo.channels];
          if (++p.phase >= m_sample.size() / m_sfInfo.channels)
            p = {0, false};
        }
    }
    return 0;
  }
  Resampler::Resampler(Memory::SimplePool* pool) : Module(pool) { }
  int Resampler::Process(Count nFrames) { return 0; }
};
