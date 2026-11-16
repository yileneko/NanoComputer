#pragma once

#include "memory.h"
#include <stdint.h>
#include <string>
#include <unordered_map>

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
// ADD_REG        reg,   reg   0x0B
// ADD_ADDR       addr,  imm   0x0C
// ADD_ADDR_REG   addr,  reg   0x0D
// ADD_REG_ADDR   reg,   addr  0x0E
// SUB_REG        reg,   reg   0x0F
// SUB_ADDR       addr,  imm   0x10
// SUB_ADDR_REG   addr,  reg   0x11
// SUB_REG_ADDR   reg,   addr  0x12
// JZ             addr         0x13
// JC             addr         0x14
// JNC            addr         0x15
inline unordered_map<string, uint8_t> codeMap = {{"HALT", 0xFF},

                                                 // MOV 家族
                                                 {"MOV", 0x01},
                                                 {"MOV_REG", 0x07},
                                                 {"MOV_ADDR", 0x08},
                                                 {"MOV_ADDR_REG", 0x09},
                                                 {"MOV_REG_ADDR", 0x0A},

                                                 // ADD 家族
                                                 {"ADD", 0x02},
                                                 {"ADD_REG", 0x0B},
                                                 {"ADD_ADDR", 0x0C},
                                                 {"ADD_ADDR_REG", 0x0D},
                                                 {"ADD_REG_ADDR", 0x0E},

                                                 // SUB 家族
                                                 {"SUB", 0x03},
                                                 {"SUB_REG", 0x0F},
                                                 {"SUB_ADDR", 0x10},
                                                 {"SUB_ADDR_REG", 0x11},
                                                 {"SUB_REG_ADDR", 0x12},

                                                 // 控制流与比较
                                                 {"JNZ", 0x04},
                                                 {"JMP", 0x05},
                                                 {"CMP", 0x06},
                                                 {"JZ", 0x13},
                                                 {"JC", 0x14},
                                                 {"JNC", 0x15}};

inline unordered_map<string, int> regNameToIndex = {
    {"AL", 0}, {"BL", 1}, {"CL", 2}, {"DL", 3},
    {"AH", 4}, {"BH", 5}, {"CH", 6}, {"DH", 7}};

class Assembler {
public:
  Assembler(Memory &mem);

  void assembleProgram(const vector<string> &program);
  void assemble(string instruction, bool isDryRun);

private:
  Memory &mem;
  uint16_t loadAddress;
  unordered_map<string, uint16_t> labelMap;
};
