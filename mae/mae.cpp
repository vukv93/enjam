/** @file */
#include "mae.h"
#include <string>
#include <cmath>
using namespace std;
namespace Mae {
  double midiCps(double midiNote) {return 440.0*pow(2,(midiNote-69)/12);}
  double cpsMidi(double freq) {return 12*log(freq/220.0)/log(2.0)+57;}
  Module::Module(Memory::SimplePool* pool, Count nOut, Count nIn)
    : m_memoryPool(pool), m_inputs(nIn), m_outputs(nOut) {}
  Count Module::MemoryNeeded(Count nFrames) {
    return m_outputs.size() * nFrames;
  }
  Module::~Module() {}
  bool Module::HasInput(Count n) {
    return m_inputs.size() > n
      && m_inputs[n] != nullptr
      && m_inputs[n]->m_buffer != nullptr;
  }
  void Module::SetSampleRate(double fs) { m_fs = fs; }
  int Module::Allocate(Count nFrames) {
    for (auto& cable : m_outputs) {
      cable.m_buffer = (Sample*)m_memoryPool->Allocate(
          nFrames * sizeof(Sample));
      if (cable.m_buffer == nullptr) return -1;
    }
    return 0;
  }
  int Module::Free() { return m_memoryPool->FreeAll(); }
  Container::Container(
      Memory::SimplePool* pool,
      vector<Module*>&& plugins,
      Count nOut,
      Count nIn) :
    Module(pool, nOut, nIn),
    m_plugins(move(plugins))
  {}
  void Container::SetSampleRate(double fs) {
    m_fs = fs; for (auto& p : m_plugins) p->SetSampleRate(fs);
  }
  int Container::Allocate(Count nFrames) {
    for (auto& p : m_plugins) if (p->Allocate(nFrames)) return -1;
    return 0;
  }
  int Container::Free() { return m_memoryPool->FreeAll(); }
  Count Container::MemoryNeeded(Count nFrames) {
    Count sum = 0; for (auto& p : m_plugins) sum += p->MemoryNeeded(nFrames);
    return sum;
  }
  MaeRt::MaeRt(Module* topLevel) :
    Mae{topLevel,0},
    m_client("mae", *this)
  {
    /* Create external ports, connect to top-level module. */
    for (int i = 0; i < m_topLevel->m_inputs.size(); i++) {
      m_inputs.push_back({
          nullptr,
          {m_client, "in_" + to_string(i), Audio::Jack::Port::In}});
      m_topLevel->m_inputs[i] = &m_inputs.back();
    }
    for (int i = 0; i < m_topLevel->m_outputs.size(); i++)
      m_outputs.push_back({
          nullptr,
          {m_client, "out_" + to_string(i), Audio::Jack::Port::Out}});
    m_fs = m_client.GetSampleRate();
    m_topLevel->SetSampleRate(m_fs);
  }
  void MaeRt::ConnectDefaultOutputs() {
    int ret = 0;
    const char** portNames = jack_get_ports(
        m_client.m_jackClient,
        nullptr,
        JACK_DEFAULT_AUDIO_TYPE,
        JackPortIsInput | JackPortIsPhysical);
    auto it = portNames;
    for (auto& out : m_outputs)
      ret = jack_connect(m_client.m_jackClient, out.m_port.GetName(), *it++);
    jack_free(portNames);
  }
  int MaeRt::CallbackProcess(uint32_t nFrames) {
    /* Connect input and output buffers */
    int i = 0;
    for (auto& in : m_inputs) {
      in.m_buffer = in.m_port.GetBuffer(nFrames);
      /* @todo[240718_043535] Test input connections. */
      m_topLevel->m_inputs[i++] = &in; 
    }
    i = 0;
    for (auto& out : m_outputs)
      m_topLevel->m_outputs[i++].m_buffer = out.m_port.GetBuffer(nFrames);
    /* Delegate processing to top-level module */
    int ret;
    if ((ret = m_topLevel->Allocate(nFrames))) return ret;
    if ((ret = m_topLevel->Process(nFrames))) return ret;
    if ((ret = m_topLevel->Free())) return ret;
    return ret;
  }
  int MaeRt::CallbackSampleRate(uint32_t nFrames) {
    m_topLevel->SetSampleRate((double)nFrames);
    return 0;
  }
  int MaeRt::CallbackShutdown() { return 0; }
  MaeNrt::MaeNrt(Module* topLevel) :
    m_topLevel(topLevel) {}
  Maybe<vector<Sample>> MaeNrt::Render(
      double duration, double fs, vector<vector<Sample>>* ins, Count bufferSize) {
    Count nFrames = (Count)(duration*fs);
    Count nOut = m_topLevel->m_outputs.size(), nIn = m_topLevel->m_inputs.size();
    vector<Sample> output(nFrames * nOut);
    vector<vector<Sample>> outputBuffers;
    vector<vector<Sample>> inputBuffers;
    for (auto& out : m_topLevel->m_outputs) {
      vector<Sample> outBuf(nFrames);
      outputBuffers.push_back(move(outBuf));
    }
    for (auto& in : m_topLevel->m_inputs) {
      vector<Sample> inBuf(nFrames);
      inputBuffers.push_back(move(inBuf));
    }
    vector<Cable> inputCables;
    Count cnt = 0, nBuffers = nFrames / bufferSize, rest = nFrames % bufferSize;
    auto process = [&] (Count nFrames) {
      m_topLevel->Allocate(nFrames);
      auto inputCable = inputCables.begin();
      auto inputBuffer = inputBuffers.begin();
      for (auto& in : m_topLevel->m_inputs) {
        (*inputCable).m_buffer = (*inputBuffer++).data();
        in = &*inputCable++;
      }
      auto outputBuffer = outputBuffers.begin();
      for (auto& out : m_topLevel->m_outputs) {
        out.m_buffer = (*outputBuffer++).data();
      }
      m_topLevel->Process(nFrames);
      m_topLevel->Free();
      Count channel = 0;
      for (auto& outBuffer : outputBuffers) {
        for (Count i = 0; i < nFrames; i++) 
          output[i*nOut+channel] = outBuffer[i];
        channel++;
      }
    };
    while (cnt < nBuffers) { process(nFrames); cnt++; }
    if (rest) process(rest);
    return output;
  }
  int MaeNrt::RenderFile(const string& fileName, double duration, double fs) {
    auto output = Render(duration, fs);
    return -1;
  }
}
