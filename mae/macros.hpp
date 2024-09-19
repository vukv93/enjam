/* @todo[240909_062444] Rename to macros.hpp. */
/** @file */
#pragma once
/** Input is connected, and its buffer assigned. */
#define HAS_INPUT(inIdx) \
    (m_inputs.size() > (inIdx) \
      && m_inputs[inIdx] != nullptr \
      && m_inputs[inIdx]->m_buffer != nullptr)
/** Input sample, or an alternative. */
#define MAYBE_SAMPLE_IN_ALT(inIdx,i,alt) \
  (HAS_INPUT(inIdx) ? m_inputs[inIdx]->m_buffer[i] : (alt))
/** Input sample, or zero. */
#define MAYBE_SAMPLE_IN_0(inIdx,i) \
  MAYBE_SAMPLE_IN_ALT(inIdx,i,0)
/** Read input sample, or previous value. */
#define MAYBE_SAMPLE_IN_PREV(place,inIdx,i) \
  place = MAYBE_SAMPLE_IN_ALT(inIdx,i,place)
/** Input sample. */
#define SAMPLE_IN(idx,i) (m_inputs[idx]->m_buffer[i])
/** Output sample. */
#define SAMPLE_OUT(idx,i) (m_outputs[idx].m_buffer[i])
/** Who small? */
#define MIN(a,b) ((a < b) ? (a) : (b))
/** Who big? */
#define MAX(a,b) ((a > b) ? (a) : (b))
/** Call a module method without arguments. */
#define MODULE_METHOD_TRIG(type,method,name)\
  void mtrig_ ## name(void* mod) {\
    ((type*)mod)->method();\
  }
/** Declaration generator. */
/* @todo[240906_050727] Type unused for declarations, SISOSIG? */
#define MODULE_METHOD_TRIG_DECL(type,method,name) \
  void mtrig_ ## name(void* mod);
/** Declaration generator. */
#define MODULE_METHOD_ARG(type,method,name)\
  void marg_ ## name(void* mod, void* arg) {\
    ((type*)mod)->method(arg);\
  }
#define MODULE_METHOD_ARG_DECL(type,method,name)\
  void marg_ ## name(void* mod, void* arg);
/** Module member getter/setter pair. */
#define MODULE_MEMBER_ACCESSOR(type,member,membertype,name)\
  membertype mget_ ## name (void* mod) { \
    return ((type*)mod)->member; \
  } \
  void mset_ ## name (void* mod, void* arg) {\
    ((type*)mod)->member  = *((membertype*)arg);\
  } \
  void msetv_ ## name (void* mod, membertype arg) { \
    ((type*)mod)->member = arg; \
  }
/** Get/set declarations. */
#define MODULE_MEMBER_ACCESSOR_DECL(type,member,membertype,name)\
  membertype mget_ ## name (void* mod); \
  void mset_ ## name (void* mod, void* arg); \
  void msetv_ ## name (void* mod, membertype arg)
/** Default factory builder registration. */
#define BUILDER_DEFAULT(type) m_builders[#type] = DefaultBuilder<type>
/** Type-provided factory builder registration. */
#define BUILDER_TYPE_PROVIDED(type) m_builders[#type]= type::Build
