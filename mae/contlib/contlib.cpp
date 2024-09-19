#include <iostream>
#include "contlib.hpp"
#include "mae/macros.hpp"
using namespace std;
namespace Mae {
  namespace Contlib {
    /* Patcher */
    /* @todo[240717_050555] Make runtime operations atomic from Process() POV. */
    /* @todo[240717_051010] So, with regards to it(=m_queue)->next. */
    /* @todo[240717_052300] Make runtime operations thread-safe. */
    /* @todo[240718_022903] Refactor it, once mostly working. */
    /* @todo[240721_022057] > As now in now. */
    /* @todo[240718_033012] Use iterators for iteration. */
    Patcher::~Patcher() { RemoveAll(); }
    void Patcher::SetSampleRate(double fs) {
      m_fs = fs;
      auto m = m_queue;
      while (m != nullptr) { m->module->SetSampleRate(fs); m = m->next; }
    }
    Error Patcher::Process(Count nFrames) {
      if (m_end) { /* Non-empty queue. */
        auto m = m_queue;
        while (m != nullptr) {
          m->module->Allocate(m_memoryPool, nFrames);
          m->module->Process(nFrames);
          m = m->next;
        }
        m_poolUsage = m_memoryPool->Used();
        m_memoryPool->FreeAll();
      }
      return ErrorNone;
    }
    Patcher::Item* Patcher::Find(const string& name) {
      auto m = m_queue;
      while (m != nullptr) {
        if (m->name == name) return m;
        m = m->next;
      }
      return nullptr;
    }
    Patcher::Item* Patcher::At(Count idx) {
      auto it = m_queue; Count i = 0;
      while (i < idx && it != nullptr) { it = it->next; i++; }
      return it;
    }
    Patcher::Module* Patcher::operator[](Count idx) {
      return At(idx)->module;
    }
    Patcher::Module* Patcher::operator[]( const string& name) {
      return Find(name)->module;
    }
    Patcher::Item* Patcher::BuildItem(
        const string& type, const string& name) {
      Module* mod = m_factory->Build(type);
      if (!mod) return nullptr;
      mod->SetSampleRate(m_fs);
      return Wrap(mod, type, name);
    }
    Patcher::Item* Patcher::Wrap(
        Module* mod, const string& type, const string& name) {
      Item* item = new Item({mod, type, name});
      if (!item) { delete mod; return nullptr; }
      else return item;
    }
    Error Patcher::Append(Item* item) {
      if (!item) return ErrorSome;
      if (m_end) return Attach(item, m_end->name, After);
      else { m_queue = m_end = item; return ErrorNone; }
    }
    Error Patcher::Prepend(Item* item) {
      if (!item) return ErrorSome;
      if (m_queue) return Attach(item, m_queue->name, Before);
      else { m_queue = m_end = item; return ErrorNone; }
    }
    Error Patcher::Append(const string& type, const string& name) {
      Item* item = BuildItem(type, name);
      return Append(item);
    }
    Error Patcher::Prepend(const string& type, const string& name) {
      Item* item = BuildItem(type, name);
      return Prepend(item);
    }
    Patcher::Item* Patcher::Detach(const string& name) {
      /* Undo connections. */
      Item* item = Find(name);
      Item* dst = m_queue;
      while (dst != nullptr) {
        for (auto& out : item->module->m_outputs) {
          for (auto& in : dst->module->m_inputs) {
            if (&out == in) in = nullptr;
          }
        }
        dst = dst->next;
      }
      if (item) {
        if (item->prev) item->prev->next = item->next;
        else m_queue = item->next;
        if (item->next) item->next->prev = item->prev;
        else m_end = item->prev;
        item->next = nullptr;
        item->prev = nullptr;
        return item;
      }
      else return nullptr;
    }
    Error Patcher::Attach(
        Item* item, const std::string& targetName, Position pos) {
      if (!item) return ErrorSome;
      Item* target = Find(targetName);
      if (!target) return ErrorSome;
      switch (pos) {
        case After:
          item->prev = target;
          if (target->next) {
            target->next->prev = item;
            item->next = target->next;
          }
          else {
            item->next = nullptr;
            m_end = item;
          }
          target->next = item;
          break;
        case Before:
          item->next = target;
          if (target->prev) {
            item->prev = target->prev;
            target->prev->next = item;
          }
          else {
            item->prev = nullptr;
            m_queue = item;
          }
          target->prev = item;
          break;
        default:
          return ErrorSome;
      }
      return ErrorNone;
    }
    Error Patcher::Remove(const string& name) {
      Item* item = Detach(name);
      if (!item) return ErrorSome;
      if (item->module) delete item->module;
      delete item;
      return ErrorNone;
    }
    void Patcher::RemoveAll() {
      while (m_queue != nullptr) Remove(m_queue->name);
    }
    Error Patcher::Move(
        const std::string& src, const std::string& dst, Position pos) {
      Item* item = Detach(src);
      if (!item) return ErrorSome;
      return Attach(item, dst, pos);
    }
    Error Patcher::Connect(
        const string& src, Count srcPort,
        const string& dst, Count dstPort) {
      Item* isrc = Find(src);
      Item* idst = Find(dst);
      if (!isrc || !idst) return ErrorSome;
      if (srcPort >= isrc->module->m_outputs.size()) return ErrorSome;
      /* Create (disconnected) input module ports up to `dstPort`. */
      if (dstPort >= idst->module->m_inputs.size())
        for (Count i = idst->module->m_inputs.size(); i <= dstPort; i++)
          idst->module->m_inputs.push_back(nullptr);
      idst->module->m_inputs[dstPort] = &isrc->module->m_outputs[srcPort];
      return ErrorNone;
    }
    Error Patcher::Disconnect(const std::string& dst, Count dstPort) {
      Item* idst = Find(dst);
      if (dstPort >= idst->module->m_inputs.size()) return ErrorSome;
      idst->module->m_inputs[dstPort] = nullptr;
      return ErrorNone;
    }
    Error Patcher::Say(const string& name, Word msg) {
      Item *item = Find(name);
      if (item) return Find(name)->module->Hear(msg);
      else return ErrorSome;
    }
    void Patcher::PrintQueue(ostream& out) {
      auto it = m_queue;
      while (it != nullptr) {
        out << it->name << std::endl;
        it = it->next;
      }
    }
    void Patcher::PrintQueueDebug(ostream& outstream) {
      outstream << "modules: " << std::endl;
      auto it = m_queue, revit = m_end;
      while (it != nullptr && revit != nullptr) {
        outstream << it->name << "\t" << revit->name << std::endl;
        it = it->next, revit = revit->prev;
      }
      outstream
        << "start=" << (m_queue ? m_queue->name : "nullptr")
        << ":end=" << (m_end ? m_end->name : "nullptr")
        << std::endl;
    }
    void Patcher::PrintQueueVerbose(ostream& outstream) {
      auto it = m_queue;
      while (it != nullptr) {
        outstream
          << it->name
          << ":module=" << it->module
          << std::endl;
        it = it->next;
      }
    }
    void Patcher::PrintConnections(ostream& outstream) {
      auto dstit = m_queue;
      while (dstit != nullptr) {
        int iidx = 0;
        for (auto& in : dstit->module->m_inputs) {
          auto srcit = m_queue;
          bool found = false;
          while (srcit != dstit) {
            int oidx = 0;
            for (auto& out : srcit->module->m_outputs) {
              if (&out == in) {
                outstream << srcit->name << "[" << oidx << "]->"
                  << dstit->name << "[" << iidx << "]" << endl;
                found = true;
                break;
              }
              oidx++;
            }
            if (found) break;
            srcit = srcit->next;
          }
          if (!found) printf("nil->%s[%d]\n", dstit->name.c_str(), iidx);
          iidx++;
        }
        dstit = dstit->next;
      }
    }
    ModuleFactory::ModuleFactory() {
      BUILDER_DEFAULT(Patcher);
    }
  }
}
