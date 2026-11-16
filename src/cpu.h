#pragma once

#include "memory.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

inline unordered_map<string, uint8_t> codeMap = {{"HALT", 0xFF}, {"MOV", 0x01},
                                                 {"ADD", 0x02},  {"SUB", 0x03},
                                                 {"JNZ", 0x04},  {"JMP", 0x05}};

inline unordered_map<string, int> regNameToIndex = {
    {"AL", 0}, {"BL", 1}, {"CL", 2}, {"DL", 3},
    {"AH", 4}, {"BH", 5}, {"CH", 6}, {"DH", 7}};

enum class OperandType { REG, IMM, MEM };

struct Operand {
  OperandType type;
  uint8_t value;
  uint16_t mem;
};

struct Instruction {
  uint8_t opcode;
  vector<Operand> operands = {};
};

class CPU {
public:
  CPU(Memory &mem);

  bool getHaltState();
  string getCPUState();
  void printState();
  void step();
  void addInstruction(string instruction);
  void runInstruction(Instruction ins);

private:
  Memory &mem;
  string cpuState;
  bool isHalt;
  vector<Instruction> instructionStream;
  size_t pc;
  uint8_t ir;
  vector<uint8_t> reg;
  bool zeroFlag;

  uint8_t readOperandValue(const Operand &op);
  void writeOperandValue(const Operand &op, uint8_t value);
};
