#include <iostream>
#include <unistd.h>
#include "mae/mae.hpp"
#include "mae/backend/mae-jack.hpp"
#include "mae/contlib/contlib.hpp"
#include "mae/modlib/modlib.hpp"
using Pool = Memory::SimplePool;
using Count = Mae::Count;
using namespace std;
using namespace Mae::Contlib;
using namespace Mae::Modlib;
using namespace Mae::Jack;
/* Module factory. */
struct TestFactory : Mae::Factory {
  TestFactory() {
    m_delegates.push_back(new Mae::Modlib::ModuleFactory());
    m_delegates.push_back(new Mae::Contlib::ModuleFactory());
  }
  ~TestFactory() {
    for (auto& d : m_delegates)
      delete d;
  }
};
int main(int argc, char** argv) {
  (void)argc, (void) argv;
  /* Setup. */
  Memory::SimplePool pool(1<<20);
  Memory::SimplePool ipool(1<<20);
  TestFactory factory;
  Patcher *lp = new Patcher;
  lp->m_memoryPool = &pool;
  lp->m_factory = &factory;
  lp->m_outputs.push_back({nullptr});
  lp->Append("SinOsc","sin0");
  lp->Append("VCA","vca0");
  lp->Append("Amp","outamp");
  lp->Append("AudioOut","out0");
  lp->Connect("sin0", 0, "outamp", 0);
  lp->Say("outamp", 20);
  lp->Say("sin0", 72);
  Mae::Jack::MaeRt mae(lp);
  auto& jc = mae.m_client;
  lp->SetSampleRate(mae.m_client.GetSampleRate());
  lp->Append(Patcher::Wrap(new AudioOut(jc,"ol"),"AudioOut","ol"));
  lp->Append(Patcher::Wrap(new AudioOut(jc,"or"),"AudioOut","or"));
  lp->Prepend(Patcher::Wrap(new AudioIn(jc,"i1"),"AudioOut","i1"));
  /* @todo[240919_091321] MIDI modules should be able to share the port. */
  lp->Prepend(Patcher::Wrap(new MidiInNote(jc,"n1"),"MidiNoteIn", "mn1"));
  lp->Prepend(Patcher::Wrap(new MidiInNote(jc,"cc1"),"MidiNoteIn", "mcc1"));
  lp->Connect("outamp",0,"ol",0);
  lp->Connect("outamp",0,"ol",0);
  lp->Connect("outamp",0,"or",0);
  /* Debug. */
  auto sin = (SinOsc*)(*lp)["sin0"];
  auto amp = (Amp*)(*lp)["outamp"];
  cout
    << &pool << endl
    << &ipool << endl
    << sin << endl
    << amp << endl;
  lp->PrintConnections(cout);
  /* Run test. */
  mae.m_client.Start();
  Count niter = 10, titer = 1;
  for (Count i = 0; i < niter; i++) {
    cout
      << "iter:" << i << ";"
      << "pool:" << lp->m_poolUsage << endl;
    sleep(titer);
  }
  mae.m_client.Stop();
  delete lp; // To delete modules before backend is deinitialized for Valgrind.
  cout << "Done" << endl;
  return 0;
}
