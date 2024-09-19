#include "mae.hpp"
#include <string>
#include <cmath>
using namespace std;
using Pool = Memory::SimplePool;
namespace Mae {
  Sample midiCps(Sample midiNote) {return 440.0*pow(2,(midiNote-69)/12);}
  Sample cpsMidi(Sample freq) {return 12*log(freq/220.0)/log(2.0)+57;}
  Module::Module() : m_inputs(0), m_outputs(0) {}
  Module::Module(Arg* arg) :
    m_inputs(arg->nInputs),
    m_outputs(arg->nOutputs) {}
  Module::Module(const Arg& arg) :
    m_inputs(arg.nInputs),
    m_outputs(arg.nOutputs) {}
  Error Module::Receive(void* msg) {
    if(m_receiver) return m_receiver(this, msg);
    else return ErrorNone;
  }
  Error Module::Hear(Word msg) { (void)msg; return ErrorNone; }
  void Module::SetSampleRate(double fs) { m_fs = fs; }
  Error Module::Allocate(Pool* pool, Count nFrames) {
    for (auto& cable : m_outputs) {
      cable.m_buffer = (Sample*)pool->Allocate(
          nFrames * sizeof(Sample));
      if (cable.m_buffer == nullptr) return ErrorSome;
    }
    return ErrorNone;
  }
  Module* Factory::Build(const std::string &type) {
    for (auto& b : m_builders) if (b.first == type)
      return b.second();
    for (auto& delegate : m_delegates)
      if (auto m = delegate->Build(type))
        return m;
    return nullptr;
  }
}
using namespace Mae;
/* C API implementation. */
#ifdef __cplusplus
extern "C" {
#endif
  void container_set_factory(void* cont, void* factory) {
    ((Container*)cont)->m_factory = (Factory*)factory;
  }
#ifdef __cplusplus
}
#endif
