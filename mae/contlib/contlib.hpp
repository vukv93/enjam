/** @file */
#pragma once
#include "mae/mae.hpp"
#include <optional>
#include <string>
#include <vector>
#include <iostream>
namespace Mae {
  /** A collection of useful module containers. */
  namespace Contlib {
    /** Interactive doubly-linked module container. */
    struct Patcher : Container {
      /** Queue element. */
      struct Item {
        Module* module;
        std::string type; 
        std::string name;
        Item* next = nullptr;
        Item* prev = nullptr;
      };
      enum Position { After = 0, Before };
      Item* m_queue = nullptr;
      Item* m_end = nullptr;
      Count m_poolUsage = 0;
      std::vector<Item*> m_orphans;
      ~Patcher();
      virtual void SetSampleRate(double fs);
      virtual Error Process(Count nFrames);
      /** Access a module wrapper by name. */
      Item* Find(const std::string& name);
      /** Access a module at position in queue. */
      Item* At(Count idx);
      /** Access a module by name. */
      Module* operator[](const std::string& name);
      /** Access a module at position in queue. */
      Module* operator[](Count idx);
      /** Build module and return item wrapper. */
      Item* BuildItem(const std::string& type, const std::string& name);
      /** Wrap an externally created module in an Item. */
      static Item* Wrap(
          Module* mod, const std::string& type, const std::string& name);
      /** Detach queue item. */
      Item* Detach(const std::string& name);
      /** (Re)Attach queue item. */
      Error Attach(Item* item, const std::string& targetName, Position pos);
      /** Add existing to queue front. */
      Error Prepend(Item* item);
      /** Add existing to queue back. */
      Error Append(Item* item);
      /** Create and add module to queue front. */
      Error Prepend(const std::string& type, const std::string& name);
      /** Create and add module to queue end. */
      Error Append(const std::string& type, const std::string& name);
      /** Move module named src after dst. */
      Error Move(const std::string& src, const std::string& dst, Position pos);
      /** Remove module by name. */
      Error Remove(const std::string& name);
      /** Remove all modules. */
      void RemoveAll();
      /** Connect ports by module name and port index.
       * If the destination input port does not exist, this function will
       * allocate ports up to provided index. Intermediate inputs will 
       * not be connected (`nullptr`). */
      Error Connect(
          const std::string& src, Count srcPort,
          const std::string& dst, Count dstPort);
      /** Disconnect an input port. Will not remove any ports. */
      Error Disconnect(const std::string& dst, Count dstPort);
      /** Say a Word to a module. */
      Error Say(const std::string& name, Word msg);
      /** Utility. */
      void PrintQueue(std::ostream& out);
      /** Utility. */
      void PrintQueueDebug(std::ostream& out);
      /** Utility. */
      void PrintQueueVerbose(std::ostream& out);
      /** Utility. */
      void PrintConnections(std::ostream& out);
    };
    /** Contlib module factory. */
    struct ModuleFactory : Factory { ModuleFactory(); };
  }
}
