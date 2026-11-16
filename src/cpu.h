#pragma once

#include "assembler.h"
#include "memory.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class CPU {
public:
  CPU(Memory &mem);

  bool getHaltState();
  string getCPUState();
  void printState();
  void setPC(uint16_t val);
  void step();

private:
  Memory &mem;
  string cpuState;
  bool isHalt;
  size_t pc;
  uint8_t ir;
  vector<uint8_t> reg;
  bool zeroFlag;
  bool carryFlag;
};
