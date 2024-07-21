#include "mae-contlib.h"
namespace Mae {
  /** Linear module container. */
  SimpleMonoStrip::SimpleMonoStrip(
      Memory::SimplePool* pool, std::vector<Module*>&& plugins) :
    Container(pool,std::move(plugins),1,0) {}
  void SimpleMonoStrip::SetSampleRate(double fs) {
    for (auto& m : m_plugins) m->SetSampleRate(fs);
  }
  int SimpleMonoStrip::Process(Count nFrames) {
    int ret = 0;
    if (m_plugins.empty()) return 0;
    /* Setup, connect inputs. */
    auto current = m_plugins.begin();
    auto next = ++m_plugins.begin();
    if (!(*current)->m_inputs.empty())
      (*current)->m_inputs[0] = m_inputs[0];
    /* Process plugins sequentially, allocate intermediate buffers. */
    while (next != m_plugins.end()) {
      auto c = *current; auto n = *next;
      if ((ret = c->Allocate(nFrames))) return ret;
      if ((ret = c->Process(nFrames))) return ret;
      n->m_inputs[0] = &c->m_outputs[0];
      current++; next++;
    }
    /* Last module generates output. */
    (*current)->m_outputs[0].m_buffer = m_outputs[0].m_buffer;
    ret = (*current)->Process(nFrames);
    /* Free memory. This should be improved. */
    m_memoryPool->FreeAll();
    return ret;
  }
  LinearPatcher::ModuleWrapper::ModuleWrapper(
      Module* module, const std::string name) :
    module(module), name(name)
  {}
  LinearPatcher::LinearPatcher(Memory::SimplePool* pool, Count nOut, Count nIn) :
    Container(pool,{},nOut,nIn) {}
  void LinearPatcher::SetSampleRate(double fs) {
    auto m = m_queue;
    while (m != nullptr) { m->module->SetSampleRate(fs); m = m->next; }
  }
  /** Audio processing callback. */
  int LinearPatcher::Process(Count nFrames) {
    ModuleWrapper* m = m_queue;
    while (m != nullptr) { m->module->Allocate(nFrames); m = m->next; }
    if (m_end)
      for (Count i = 0;
          i < m_end->module->m_outputs.size() && i < m_outputs.size();
          i++)
        m_end->module->m_outputs[i].m_buffer = m_outputs[i].m_buffer;
    m = m_queue;
    while (m != nullptr) { m->module->Process(nFrames); m = m->next; }
    m_memoryPool->FreeAll();
    return 0;
  }
  /** Access a module at position in queue. */
  LinearPatcher::ModuleWrapper* LinearPatcher::At(Count idx) {
    auto it = m_queue; int i = 0;
    while (i < idx && it != nullptr) { it = it->next; i++; }
    return it;
  }
  /** Access a module at position in queue. */
  LinearPatcher::ModuleWrapper* LinearPatcher::operator[](Count idx) { return At(idx); }
  /** Get module position in queue. */
  LinearPatcher::Maybe<Count> LinearPatcher::Find(ModuleWrapper* mod) {
    Maybe<Count> idx = {};
    Count i = 0;
    auto it = m_queue;
    while (it != nullptr) {
      if (it == mod) { idx = i; break; }
      it = it->next; i++;
    }
    return idx;
  }
  /** Add module to the list at position, start by default. */
  void LinearPatcher::Add(Module* newModule, const std::string& name, Count position) {
    ModuleWrapper* newWrap = new ModuleWrapper(newModule,name);
    if (m_queue == nullptr) {
      /* Add to empty. */
      m_end = m_queue = newWrap;
    }
    else {
      if (position == 0) {
        /* Add to start. */
        newWrap->next = m_queue;
        m_queue->prev = newWrap;
        m_queue = newWrap;
      }
      else {
        ModuleWrapper* target = At(position);
        if (target) {
          /* Add at position. */
          ModuleWrapper* prev = target->prev;
          newWrap->next = target;
          newWrap->prev = prev;
          target->prev = newWrap;
          prev->next = newWrap;
        }
        else {
          /* Add to end. NOTE: Done for any invalid position. */
          newWrap->prev = m_end;
          m_end->next = newWrap;
          m_end = newWrap;
        }
      }
    }
  }
  /** Remove module, undoing all existing connections from it. */
  void LinearPatcher::Remove(Count position) {
    ModuleWrapper* target = At(position);
    if (!target) return;
    if (target->prev) target->prev->next = target->next;
    else m_queue = target->next;
    if (target->next) target->next->prev = target->prev;
    else m_end = target->prev;
    delete target;
  }
  /** Move module from position src to dst. */
  /* @todo[240717_061309] Mind the connections. Later. */
  void LinearPatcher::Move(Count src, Count dst) {
    if (src != dst) {
      ModuleWrapper* mod = At(src);
      ModuleWrapper* target = At(dst);
      /* @todo[240717_061653] Handle start and end src/dst. Check. Test. */
      if (mod->next) mod->next->prev = mod->prev;
      if (mod->prev) mod->prev->next = mod->next;
      if (src > dst) {
        if (!mod->next) m_end = mod->prev;
        mod->prev = target->prev;
        mod->next = target;
        if (target->prev) target->prev->next = mod;
        target->prev = mod;
        if (!dst) m_queue = mod;
      }
      else {
        if (!src) m_queue = mod->next;
        mod->prev = target;
        mod->next = target->next;
        if (target->next) target->next->prev = mod;
        else m_end = mod;
        target->next = mod;
      }
    }
  }
  /** Connect output port of the source to input port of the destination. */
  void LinearPatcher::Connect(
      Count srcIdx, Count srcPort,
      Count dstIdx, Count dstPort) {
    if (srcIdx >= dstIdx) return;
    ModuleWrapper* src = At(srcIdx);
    ModuleWrapper* dst = At(dstIdx);
    if (src && dst
        && srcPort < src->module->m_outputs.size()
        && dstPort < dst->module->m_inputs.size()) 
      dst->module->m_inputs[dstPort] =
        &src->module->m_outputs[srcPort];
  }
  /** Disconnect input of a module (latching last value?). */
  int LinearPatcher::Disconnect(Count dstIdx, Count dstPort) {
    ModuleWrapper* dst = At(dstIdx);
    if(dstPort >= dst->module->m_inputs.size()) return -1;
    else {
      dst->module->m_inputs[dstPort] = nullptr;
      return 0;
    }
  }
  /** Utility. */
  void LinearPatcher::PrintQueue() {
    auto it = m_queue;
    while (it != nullptr) {
      std::cout << it->name << std::endl;
      it = it->next;
    }
  }
  /** Utility. */
  void LinearPatcher::PrintQueueDebug() {
    std::cout << "modules: " << std::endl;
    auto it = m_queue, revit = m_end;
    while (it != nullptr && revit != nullptr) {
      std::cout << it->name << "\t" << revit->name << std::endl;
      it = it->next, revit = revit->prev;
    }
    std::cout
      << "start=" << (m_queue ? m_queue->name : "nullptr")
      << ":end=" << (m_end ? m_end->name : "nullptr")
      << std::endl;
  }
  /** Utility. */
  void LinearPatcher::PrintQueueVerbose() {
    auto it = m_queue;
    while (it != nullptr) {
      std::cout
        << it->name
        << ":module=" << it->module
        << std::endl;
      it = it->next;
    }
  }
  /** Utility. */
  void LinearPatcher::PrintConnections() {
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
              printf("%s[%d]->%s[%d]\n",
                  srcit->name.c_str(), oidx,
                  dstit->name.c_str(), iidx);
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
}
