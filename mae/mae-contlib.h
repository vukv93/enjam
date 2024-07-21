/** @file */
#pragma once
#include "mae.h"
#include <optional>
#include <iostream>
#include <string>
#include <cstdio>
namespace Mae {
  /** Linear module container. */
  struct SimpleMonoStrip : Container {
    SimpleMonoStrip(Memory::SimplePool* pool, std::vector<Module*>&& plugins);
    void SetSampleRate(double fs);
    int Process(Count nFrames);
  };
  /** Interactive doubly-linked module container. */
  /* @todo[240717_050555] Make runtime operations atomic from Process() POV. */
  /* @todo[240717_051010] So, with regards to it(=m_queue)->next. */
  /* @todo[240717_052300] Make runtime operations thread-safe. */
  /* @todo[240718_022903] Refactor it, once mostly working. */
  /* @todo[240721_022057] > As now in now. */
  /* @todo[240718_033012] Use iterators for iteration. */
  struct LinearPatcher : Container {
    struct ModuleWrapper {
      ModuleWrapper(Module* module, const std::string name);
      Module* module;
      ModuleWrapper* next = nullptr;
      ModuleWrapper* prev = nullptr;
      std::string name;
    };
    ModuleWrapper* m_queue = nullptr;
    ModuleWrapper* m_end = nullptr;
    LinearPatcher(Memory::SimplePool* pool, Count nOut = 1, Count nIn = 0);
    virtual int Allocate(Count nFrames);
    virtual void SetSampleRate(double fs);
    /** Audio processing callback. */
    virtual int Process(Count nFrames);
    /** Access a module at position in queue. */
    ModuleWrapper* At(Count idx);
    /** Access a module at position in queue. */
    ModuleWrapper* operator[](Count idx);
    /** Get module position in queue. */
    Maybe<Count> Find(ModuleWrapper* mod);
    /** Add module to the list at position, start by default. */
    void Add(Module* newModule, const std::string& name, Count position = 0);
    /** Remove module, undoing all existing connections from it. */
    void Remove(Count position = 0);
    /** Move module from position src to dst. */
    void Move(Count src, Count dst);
    /** Connect output port of the source to input port of the destination. */
    void Connect(
        Count srcIdx, Count srcPort,
        Count dstIdx, Count dstPort);
    /** Disconnect input of a module (latching last value?). */
    int Disconnect(Count dstIdx, Count dstPort);
    /** Utility. */
    void PrintQueue();
    /** Utility. */
    void PrintQueueDebug();
    /** Utility. */
    void PrintQueueVerbose();
    /** Utility. */
    void PrintConnections();
  };
  struct Factory {};
}
