#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

class Memory {
public:
  Memory();

  bool checkAddrAvailable(uint16_t addr);
  uint8_t read8(uint16_t addr);
  void write8(uint16_t addr, uint8_t value);

private:
  vector<uint8_t> memory;
};
