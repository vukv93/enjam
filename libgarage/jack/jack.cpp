#include <jack/jack.h>
#include <cmath>
#include <stdexcept>
#include "jack.hpp"
// @todo[240619_024050] Error handling.
/* @todo[240721_013359] And a lot more. */
using namespace std;
static int jackProcessCallbackWrapper(jack_nframes_t n, void* arg) {
  return ((Audio::Jack::ICallback*)arg)->CallbackProcess(n);
}
static int jackSampleRateCallbackWrapper(jack_nframes_t fs, void* arg) {
  return ((Audio::Jack::ICallback*)arg)->CallbackSampleRate(fs);
}
static void jackShutdownCallbackWrapper(void* arg) {
  ((Audio::Jack::ICallback*)arg)->CallbackShutdown();
}
Audio::Jack::Client::Client(const string& name, ICallback& callback) {
  m_jackClient = jack_client_open(name.c_str(), JackNullOption, nullptr);
  if (m_jackClient == nullptr)
    throw runtime_error("Couldn't open client");
  if (jack_set_process_callback(
          m_jackClient, jackProcessCallbackWrapper, (void*)&callback)
        || jack_set_sample_rate_callback(
          m_jackClient, jackSampleRateCallbackWrapper, (void*)&callback))
    throw runtime_error("Could not set a callback");
  jack_on_shutdown(
      m_jackClient, jackShutdownCallbackWrapper, (void*)&callback);
}
Audio::Jack::Client::~Client() { jack_client_close(m_jackClient); }
int Audio::Jack::Client::Start() { return jack_activate(m_jackClient); }
int Audio::Jack::Client::Stop() { return jack_deactivate(m_jackClient); }
double Audio::Jack::Client::GetSampleRate() {
  return static_cast<double>(jack_get_sample_rate(m_jackClient));
}
Audio::Jack::Port::Port(
    Client& client,
    const std::string& name,
    Direction direction,
    Type type) {
  const char* jack_port_type;
  unsigned long jack_port_flags = 0;
  switch (direction) {
    case Out: jack_port_flags |= JackPortIsOutput; break;
    case In: jack_port_flags |= JackPortIsInput; break;
    default: throw runtime_error("Unknown port direction");
  }
  switch (type) {
    case Audio: jack_port_type = JACK_DEFAULT_AUDIO_TYPE; break;
    case Midi:  jack_port_type = JACK_DEFAULT_MIDI_TYPE; break;
    default: throw runtime_error("Unknown port type");
  }
  m_client = &client;
  m_jackPort = jack_port_register(
      client.m_jackClient,
      name.c_str(),
      jack_port_type,
      jack_port_flags,
      0);
  if (m_jackPort == nullptr)
    throw runtime_error("Could not create port");
}
Audio::Jack::Port::Port(Port&& old) :
  m_client(old.m_client),
  m_jackPort(old.m_jackPort)
{
  old.m_client = nullptr;
  old.m_jackPort = nullptr;
}
Audio::Jack::Port::~Port() {
  if (m_client != nullptr && m_jackPort != nullptr)
    jack_port_unregister(m_client->m_jackClient, m_jackPort);
}
void* Audio::Jack::Port::GetBuffer(size_t nFrames) {
  return jack_port_get_buffer(m_jackPort, nFrames);
}
const char* Audio::Jack::Port::GetName() {
  return jack_port_name(m_jackPort);
}
Audio::Jack::AudioPort::AudioPort(
    Client& client,
    const std::string& name,
    Direction direction) :
  Port(client, name, direction, Audio)
{}
Audio::Jack::AudioPort::AudioPort(AudioPort&& old) : Port(std::move(old)) {}
Audio::Jack::Sample* Audio::Jack::AudioPort::GetBuffer(size_t nFrames) {
  return (Audio::Jack::Sample*)jack_port_get_buffer(m_jackPort, nFrames);
}
Audio::Jack::MidiPort::MidiPort(
    Client& client,
    const std::string& name,
    Direction direction) :
  Port(client, name, direction, Midi)
{}
Audio::Jack::MidiPort::MidiPort(MidiPort&& old) : Port(std::move(old)) {}
Audio::Jack::MidiEvent* Audio::Jack::MidiPort::GetBuffer(size_t nFrames) {
  return (Audio::Jack::MidiEvent*)jack_port_get_buffer(m_jackPort, nFrames);
}
Audio::Jack::Count Audio::Jack::MidiPort::GetNEvents(MidiEvent* eventBuffer) {
  return jack_midi_get_event_count(eventBuffer);
}
int Audio::Jack::MidiPort::GetEvent(
    MidiEvent* ev, Count n, MidiEvent* eventBuffer) {
  return jack_midi_event_get(ev, eventBuffer, n);
}
