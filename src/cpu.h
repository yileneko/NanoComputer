#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

inline unordered_map<string, uint8_t> codeMap = {
    {"HALT", 0xFF}, {"MOV", 0x01}, {"ADD", 0x02}, {"SUB", 0x03}, {"JNZ", 0x04}};

inline unordered_map<string, int> regNameToIndex = {
    {"AL", 0}, {"BL", 1}, {"CL", 2}, {"DL", 3},
    {"AH", 4}, {"BH", 5}, {"CH", 6}, {"DH", 7}};

struct Operand {
  bool isReg;
  uint8_t value;
};

struct Instruction {
  uint8_t opcode;
  vector<Operand> operands = {};
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
  bool zeroFlag;
};
