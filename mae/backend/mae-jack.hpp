/** @file */
#pragma once
#include <vector>
#include "mae/mae.hpp"
#include "libgarage/jack/jack.hpp"
namespace Mae {
  /** JACK audio connection kit real-time backend for Mae. */
  namespace Jack {
    struct ExtPortArg { void* pool; const char* name; void* client; };
    /** External audio input port. */
    struct AudioIn : Module {
      Audio::Jack::AudioPort m_port;
      AudioIn(Audio::Jack::Client& client, const std::string& name) :
        Module({nullptr,1,0}), m_port(client, name, Audio::Jack::Port::In) {}
      Error Process(Count nFrames);
    };
    /** External audio output port. */
    struct AudioOut : Module {
      Audio::Jack::AudioPort m_port;
      AudioOut(Audio::Jack::Client& client, const std::string& name) :
        Module({nullptr,1,0}), m_port(client, name, Audio::Jack::Port::Out) {}
      Error Process(Count nFrames);
    };
    /** External MIDI note gate, velocity, and frequency generator. */
    struct MidiInNote : Module {
      using Arg = ExtPortArg;
      Audio::Jack::MidiPort m_port;
      Count m_channel = 0;
      bool m_gate = false;
      Sample m_freq = 440;
      Sample m_vel = 0.0;
      MidiInNote(Audio::Jack::Client& client, const std::string& name) :
        Module({nullptr,3,0}), m_port(client, name, Audio::Jack::Port::In) {}
      Error Process(Count nFrames);
    };
    /** External MIDI input port CC message Hear dispatcher. */
    struct MidiInCCHearDispatcher : Module {
      using Arg = ExtPortArg;
      Audio::Jack::MidiPort m_port;
      Count m_channel = 0;
      std::map<Count, Module*> m_hearMap; /**< CC to target Module mappings. */
      MidiInCCHearDispatcher(
          Audio::Jack::Client& client, const std::string& name) :
        Module({nullptr,0,0}), m_port(client, name, Audio::Jack::Port::In) {}
      Error Process(Count nFrames);
    };
    /** External audio interface. */
    struct ExternCable : Cable { Audio::Jack::AudioPort m_port; };
    /** Online mode engine. */
    struct MaeRt : Mae, Audio::Jack::ICallback {
      Audio::Jack::Client m_client;
      MaeRt(Module* topLevel);
      virtual int CallbackProcess(uint32_t nFrames);
      virtual int CallbackSampleRate(uint32_t nFrames);
      virtual int CallbackShutdown();
    };
  }
}
