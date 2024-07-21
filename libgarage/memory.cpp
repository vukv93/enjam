#include <iostream>
#include "memory.h"
using namespace std;
/** First try */
namespace Memory {
  SimplePool::SimplePool(Count size)
    : m_size(size),
    m_block(new uint8_t[m_size]),
    m_unused(m_block)
  {}
  SimplePool::~SimplePool() { delete[] m_block; }
  uint8_t* SimplePool::Allocate(Count size) {
    if (m_unused + size > m_block + m_size) return nullptr;
    else {
      uint8_t* blockStart = m_unused; 
      m_unused += size;
      return blockStart;
    }
  }
  int SimplePool::FreeAll() {
    m_unused = m_block;
    return 0;
  }
}
/** Second try */
/* @todo[240629_074050] Implement better allocation algorithm. */
namespace Memory {
  PoolV2::PoolV2(Count size, Count tableSize)
    : m_size(size),
    m_block(new uint8_t[m_size]),
    m_freeTable(tableSize), m_usedTable(tableSize)
  {}
  PoolV2::~PoolV2() { delete[] m_block; }
  Element PoolV2::Allocate(Count size) {
    for (auto& e : m_freeTable) {
    }
    return {};
  }
}
