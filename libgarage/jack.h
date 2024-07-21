/** @file */
#pragma once
#include <string>
#include <jack/midiport.h>
#include <jack/jack.h>
namespace Audio {
  struct Jack {
    using Sample = jack_default_audio_sample_t;
    using MidiEvent = jack_midi_event_t;
    struct ICallback {
      double m_sampleRate;
      virtual int CallbackProcess(uint32_t nFrames) = 0;
      virtual int CallbackSampleRate(uint32_t nFrames) = 0;
      virtual int CallbackShutdown() = 0;
    };
    struct MockCallback : ICallback {
      virtual int CallbackProcess(uint32_t nFrames) { return 0; }
      virtual int CallbackSampleRate(uint32_t nFrames) { return 0; }
      virtual int CallbackShutdown() {return 0; }
    };
    struct Client {
      jack_client_t* m_jackClient;
      Client(const std::string& name, ICallback& callback);
      ~Client();
      int Start();
      int Stop();
      double GetSampleRate();
    };
    struct Port {
      enum Direction {Out, In};
      enum Type {Audio, Midi};
      const Client* m_client;
      jack_port_t* m_jackPort;
      Port(
          Client& client,
          const std::string& name,
          Direction direction = Out,
          Type type = Audio);
      Port(Port&& old);
      ~Port();
      void* GetBuffer(size_t nFrames);
      const char* GetName();
    };
    struct AudioPort : Port {
      AudioPort(
          Client& client,
          const std::string& name,
          Direction direction = Out);
      AudioPort(AudioPort&& old);
      Sample* GetBuffer(size_t nFrames);
    };
    /* @todo[240629_074149] Handle MIDI. */
    struct MidiPort : Port {
      MidiPort(
          Client& client,
          const std::string& name,
          Direction direction = Out);
      MidiEvent* GetBuffer(size_t nFrames);
    };
  };
};
