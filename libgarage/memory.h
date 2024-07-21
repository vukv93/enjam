/** @file */
#pragma once
#include <cstdint>
#include <vector>
/** Memory management utilities. */
namespace Memory {
  /** For counting things. */
  using Count = unsigned long;
  /** Memory allocation unit. */
  struct Element {
    Count size;
    void* address;
  };
  /** Memory allocation pool. */
  struct SimplePool {
    Count m_size;
    uint8_t* m_block;
    uint8_t* m_unused;
    SimplePool(Count size);
    ~SimplePool();
    uint8_t* Allocate(Count size);
    int FreeAll();
  };
  /** TBD */
  struct PoolV2 {
    Count m_size;
    uint8_t* m_block;
    std::vector<Element> m_freeTable;
    std::vector<Element> m_usedTable;
    PoolV2(Count size, Count tableSize);
    ~PoolV2();
    Element Allocate(Count size);
    int Free(Element& element);
    int Free(void* address);
    int FreeAll();
  };
}
/* @todo[240624_163903] Read "Algorithms and data structures" book. */
/* @todo[240624_164000] Find some reference material on real-time allocation. */
/* @todo[240624_163945] Implement what makes sense. */
