/**
 * @file
 * @mainpage Enjam — Modular environment.
 * @section Introduction
 *
 * It can play sound. If you have a properly prepared Debian system.
 */
#include <iostream>
#include <thread>
#include <unistd.h>
#include "libgarage/main.h"
#include "libgarage/jack.h"
#include "libgarage/memory.h"
#include "mae/mae.h"
#include "mae/mae-modlib.h"
#include "mae/mae-contlib.h"
using namespace std;
/** Main application class. */
struct Enjam : Main::IApp {
  using Count = Mae::Count;
  using Sample = Audio::Jack::Sample;
  double m_duration;
  /* @todo[240718_093458] Add global pool and top-level LinearPatcher.
   * Then use it as an interface to the system in all threads (Score,
   * Monitor). */
  Enjam() {
    using namespace clipp;
    /* @todo[240629_072404] Add connect to specified JACK ports option. */
    /* @todo[240629_072643] Add non-realtime rendering option. */
    /* @todo[240721_014753] Add record flag, require output file name. */
    m_cli = group(number("jam duration", m_duration));
  }
  void Score() {}
  void Monitor() {}
  void Run() {
    cout << "Jam duration: " << m_duration << "s" << endl;
    Memory::SimplePool pool(1<<20);
    Mae::LinearPatcher lp(&pool,2);
    Mae::SinOsc sin1(&pool);
    Mae::SinOsc sin2(&pool);
    Mae::SawOsc saw1(&pool);
    Mae::SawOsc saw2(&pool);
    Mae::VCA vca1(&pool);
    Mae::EnvAR env1(&pool);
    env1.m_release = 1; env1.m_attack = .02;
    Mae::Pan pan1(&pool);
    env1.m_trigger = true;
    lp.Add(&pan1,"pan1");
    lp.Add(&vca1,"vca1");
    lp.Add(&env1,"env1");
    lp.Add(&sin1,"sin1");
    lp.Connect(0,0,2,0);
    lp.Connect(1,0,2,1);
    lp.Connect(2,0,3,0);
    cout << "module queue:" << endl; lp.PrintQueueVerbose();
    cout << "connections:" << endl; lp.PrintConnections();
    Mae::Mae mae(&lp);
    mae.m_client.Start();
    mae.ConnectDefaultOutputs();
    cout << fixed; cout.precision(2);
    thread scoreThread(&Enjam::Score, this);
    double t = 0, dt = 1;
    while ((t += dt) < m_duration) {
      /* @todo[240629_074402] Implement FTXUI user interface. */
      /* @todo[240721_011909] Implement ImGui user interface. */
      /* @todo[240718_092147] Implement as monitoring thread. */
      cout
        << "t=" << t << ":"
        << "pool-usage=" << pool.m_unused - pool.m_block
        << endl;
      usleep(dt * 1e6);
    }
    scoreThread.join();
    mae.m_client.Stop();
  }
  void Test() {}
};
MAIN(Enjam);

