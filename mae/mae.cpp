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
  Container::Container(
      Memory::SimplePool* pool,
      std::vector<Module*>&& plugins,
      Count nOut,
      Count nIn) :
    Module(pool, nOut, nIn),
    m_plugins(std::move(plugins))
  {}
  void Container::SetSampleRate(double fs) {
    m_fs = fs; for (auto& p : m_plugins) p->SetSampleRate(fs);
  }
  Mae::Mae(Module* topLevel) :
    m_topLevel(topLevel),
    m_client("mae", *this),
    m_fs(m_client.GetSampleRate())
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
    m_topLevel->SetSampleRate(m_fs);
  }
  void Mae::ConnectDefaultOutputs() {
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
  int Mae::CallbackProcess(uint32_t nFrames) {
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
    return m_topLevel->Process(nFrames);
  }
  int Mae::CallbackSampleRate(uint32_t nFrames) {
    m_topLevel->SetSampleRate((double)nFrames);
    return 0;
  }
  int Mae::CallbackShutdown() { return 0; }
}
