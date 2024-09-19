/** @file */
#pragma once
#include <cstdint>
#include <vector>
/** Memory management utilities. */
namespace Memory {
  /** For counting things. */
  using Count = unsigned long;
  /* @todo[240817_032456] Abstract pool interface. */
  struct Pool {
    /** Get a chunk of size or nullptr. */
    virtual uint8_t* Allocate(Count size) = 0;
    /** Get number of currently allocated bytes. */
    virtual Count Used() = 0;
    /** Reset allocations. */
    virtual int FreeAll() = 0;
    /** Resize the pool. */
    virtual int Resize(Count size) = 0;
  };
  /** Simple realtime memory allocation pool. */
  struct SimplePool : Pool {
    Count m_size;
    uint8_t* m_block;
    uint8_t* m_unused;
    SimplePool(Count size);
    ~SimplePool();
    uint8_t* Allocate(Count size);
    Count Used() { return m_unused-m_block; }
    int FreeAll();
    int Resize(Count size);
  };
}
/* @todo[240624_163903] Read "Algorithms and data structures" book. */
/* @todo[240624_164000] Find some reference material on real-time allocation. */
/* @todo[240624_163945] Implement what makes sense. */
