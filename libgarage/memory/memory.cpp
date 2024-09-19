#include <iostream>
#include "memory.hpp"
using namespace std;
namespace Memory {
  SimplePool::SimplePool(Count size)
    : m_size(size),
    m_block(new uint8_t[m_size]),
    m_unused(m_block)
  {
    if (!m_block) throw runtime_error("Could not allocate pool block");
  }
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
  int SimplePool::Resize(Count size) {
    delete[] m_block;
    m_block = new uint8_t[size];
    if (m_block) { m_size = size; return 0; }
    else return -1;
  }
}
