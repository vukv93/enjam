#include <stdexcept>
#include <string.h>
#include "mae-jack.hpp"
#include "mae/macros.hpp"
using namespace std;
using namespace Audio::Jack;
namespace Mae {
  namespace Jack {
    MaeRt::MaeRt(Module* topLevel) : Mae{topLevel}, m_client("mae", *this) {
        m_fs = m_client.GetSampleRate();
        m_topLevel->SetSampleRate(m_fs);
      }
    int MaeRt::CallbackProcess(uint32_t nFrames) {
      /* Delegate processing to top-level module */
      return m_topLevel->Process(nFrames);
    }
    int MaeRt::CallbackSampleRate(uint32_t nFrames) {
      m_topLevel->SetSampleRate((double)nFrames);
      return 0;
    }
    int MaeRt::CallbackShutdown() {
      /* @todo[240918_105552] Implement server shutdown handling. */
      throw runtime_error("JACK server shut down");
    }
    Error AudioOut::Process(Count nFrames) {
      auto joutbuf = m_port.GetBuffer(nFrames);
      for (Count i = 0; i < nFrames; i++) {
        joutbuf[i] = MAYBE_SAMPLE_IN_0(0,i);
      }
      return ErrorNone;
    }
    Error AudioIn::Process(Count nFrames) {
      auto jinbuf = m_port.GetBuffer(nFrames);
      for (Count i = 0; i < nFrames; i++) {
        SAMPLE_OUT(0,i) = jinbuf[i];
      }
      return ErrorNone;
    }
    Error MidiInNote::Process(Count nFrames) {
      /* @todo[240917_145143] Get events, generate gate, velocity, and frequency
       * signals for tracked channel. */
      MidiEvent ev;
      MidiEvent* eventBuffer = m_port.GetBuffer(nFrames);
      Count nEvents = m_port.GetNEvents(eventBuffer);
      Count cntEv = 0;
      if (nEvents)
        if (m_port.GetEvent(&ev, cntEv++, eventBuffer))
          return ErrorSome;
      for (Count i = 0; i < nFrames; i++) {
        while (i == ev.time) {
          if (ev.buffer[0] == (0x90 | m_channel)) {
            m_gate = true;
            m_freq = midiCps(ev.buffer[1] & 0x7f);
            m_vel = ev.buffer[2] & 0x7f;
          }
          if (ev.buffer[0] == (0x80 | m_channel)) {
            m_gate = false; 
          }
          if (cntEv == nEvents) break;
          if (m_port.GetEvent(&ev, cntEv++, eventBuffer)) return ErrorSome;
        }
        SAMPLE_OUT(0,i) = (Sample)m_gate;
        SAMPLE_OUT(1,i) = m_freq;
        SAMPLE_OUT(2,i) = m_vel;
      }
      return ErrorNone;
    }
    Error MidiInCCHearDispatcher::Process(Count nFrames) {
      /* @todo[240917_145143] Get events, call Hear of mapped modules. */
      (void)nFrames;
      MidiEvent ev;
      MidiEvent* eventBuffer = m_port.GetBuffer(nFrames);
      Count nEvents = m_port.GetNEvents(eventBuffer);
      if (!nEvents) return ErrorNone;
      for (Count i = 0; i < nEvents; i++) {
        if (m_port.GetEvent(&ev, i, eventBuffer)) return ErrorSome;
        if (ev.buffer[0] == (0xb0 | m_channel)) {
          if (m_hearMap[ev.buffer[1]])
            m_hearMap[ev.buffer[1]]->Hear(ev.buffer[2]);
        }
      }
      return ErrorNone;
    }
  }
}
