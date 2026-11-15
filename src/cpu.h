#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

inline unordered_map<string, uint8_t> codeMap = {{"HALT", 0xFF}, {"MOV", 0x01}};

struct Instruction {
  uint8_t opcode;
  vector<uint16_t> operands = {};
};

class CPU {
public:
  CPU();

  bool getHaltState();
  string getCPUState();
  void printState();
  void step();
  void addInstruction(string instruction);
  void runInstruction(Instruction ins);

private:
  string cpuState;
  bool isHalt;
  vector<Instruction> instructionStream;
  size_t pc;
  uint8_t ir;
  vector<uint8_t> reg;
};
