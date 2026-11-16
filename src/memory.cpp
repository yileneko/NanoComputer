#include "memory.h"
#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <vector>

using namespace std;

Memory::Memory() : memory(65536, 0) {};

bool Memory::checkAddrAvailable(uint16_t addr) {
  if (addr < 0 || addr >= memory.size())
    return false;
  return true;
}

uint8_t Memory::read8(uint16_t addr) {
  if (!checkAddrAvailable(addr)) {
    cerr << "ERR: the address is not available! " << "address: " << addr
         << endl;
    return 0;
  }
  return memory[addr];
}

uint16_t Memory::read16(uint16_t addr) {
  if (!checkAddrAvailable(addr) || !checkAddrAvailable(addr + 1)) {
    cerr << "ERR: the address is not available! " << "address: " << addr
         << endl;
    return 0;
  }
  uint8_t low = memory[addr];
  uint8_t high = memory[addr + 1];
  return ((static_cast<uint16_t>(high) << 8) | low);
}

void Memory::write8(uint16_t addr, uint8_t value) {
  if (!checkAddrAvailable(addr)) {
    cerr << "ERR: the address is not available! " << "address: " << addr
         << endl;
    return;
  }
  memory[addr] = value;
}
