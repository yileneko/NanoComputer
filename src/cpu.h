#pragma once

#include "memory.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// MOV            reg,   imm   0x01
// ADD            reg,   imm   0x02
// SUB            reg,   imm   0x03
// JNZ            addr         0x04
// JMP            addr         0x05
// CMP            reg,   reg   0x06
// MOV_REG        reg,   reg   0x07
// MOV_ADDR       addr,  imm   0x08
// MOV_ADDR_REG   addr,  reg   0x09
// MOV_REG_ADDR   reg,   addr  0x0A

inline unordered_map<string, uint8_t> codeMap = {
    {"HALT", 0xFF},         {"MOV", 0x01},         {"ADD", 0x02},
    {"SUB", 0x03},          {"JNZ", 0x04},         {"JMP", 0x05},
    {"CMP", 0x06},          {"MOV_REG", 0x07},     {"MOV_ADDR", 0x08},
    {"MOV_ADDR_REG", 0x09}, {"MOV_REG_ADDR", 0x0A}};

inline unordered_map<string, int> regNameToIndex = {
    {"AL", 0}, {"BL", 1}, {"CL", 2}, {"DL", 3},
    {"AH", 4}, {"BH", 5}, {"CH", 6}, {"DH", 7}};

class CPU {
public:
  CPU(Memory &mem);

  bool getHaltState();
  string getCPUState();
  void printState();
  void setPC(uint16_t val);
  void step();
  void addInstruction(string instruction);

private:
  Memory &mem;
  string cpuState;
  bool isHalt;
  size_t pc;
  uint16_t loadAddress;
  uint8_t ir;
  vector<uint8_t> reg;
  bool zeroFlag;
  bool carryFlag;
};
