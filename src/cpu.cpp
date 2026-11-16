#include "cpu.h"
#include "memory.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <future>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

uint16_t parseMemoryAddress(const std::string &token) {
  if (token.length() < 2 || token.front() != '[' || token.back() != ']') {
    cerr << "ERR: Invalid memory format! " << "memory: " << token << endl;
  }

  std::string innerStr = token.substr(1, token.length() - 2);

  try {
    uint32_t value = std::stoul(innerStr, nullptr, 0);

    if (value > 0xFFFF) {
      cerr << "ERR: Address out of 16-bit range! " << "address: " << innerStr
           << endl;
    }

    return static_cast<uint16_t>(value);
  } catch (const std::exception &e) {
    cerr << "ERR: Failed to parse memory address! " << "address: " << innerStr
         << endl;
  }

  return 0;
}

std::string toHexString(uint8_t v) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(v);
  return oss.str();
}

bool checkMem(string str) {
  if (str[0] == '[' && str[str.size() - 1] == ']') {
    return true;
  }

  return false;
}

string getInstructionType(string instruction) {
  auto spacePos = instruction.find_first_of(" \t,");
  if (spacePos == string::npos)
    return instruction;
  return instruction.substr(0, spacePos);
}

vector<string> getInstructionOperand(string instruction) {
  vector<string> ret = {};
  size_t pos = 0;
  while (1) {
    auto start = instruction.find_first_not_of(" \t,", pos);
    if (start == string::npos)
      break;

    auto end = instruction.find_first_of(" \t,", start);
    if (end == string::npos) {
      ret.push_back(instruction.substr(start));
      break;
    }
    ret.push_back(instruction.substr(start, end - start));
    pos = end + 1;
  }
  return ret;
}

CPU::CPU(Memory &mem) : mem(mem) {
  pc = 0;
  loadAddress = 0;
  reg.clear();
  for (int i = 0; i < 8; i++) {
    reg.push_back(0);
  }
  zeroFlag = false;
  carryFlag = false;

  cpuState = "running";
  isHalt = false;
};

bool CPU::getHaltState() { return isHalt; }

string CPU::getCPUState() { return cpuState; }

void CPU::printState() {
  cout << "PC: " << pc << " " << "IR: " << toHexString(ir) << " "
       << " [ZF: " << zeroFlag << ", CF: " << carryFlag << "] "
       << "cpuState: " << cpuState << endl
       << "register: ";
  for (int i = 0; i < reg.size(); i++) {
    cout << toHexString(reg[i]) << " ";
  }
  cout << endl;
}

void CPU::setPC(uint16_t val) { pc = val; }

void CPU::step() {
  if (isHalt)
    return;
  cpuState = "running";

  ir = mem.read8(pc);
  pc++;

  switch (ir) {
  // HALT
  case 0xFF: {
    cpuState = "down";
    isHalt = true;
    break;
  }
  // MOV reg, imm
  case 0x01: {
    uint8_t dst = mem.read8(pc++);
    uint8_t imm = mem.read8(pc++);
    reg[dst] = imm;
    break;
  }
  // ADD reg, imm
  case 0x02: {
    uint8_t dst = mem.read8(pc++);
    uint8_t imm = mem.read8(pc++);
    reg[dst] = static_cast<uint8_t>(reg[dst] + imm);
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // SUB reg, imm
  case 0x03: {
    uint8_t dst = mem.read8(pc++);
    uint8_t imm = mem.read8(pc++);
    reg[dst] = static_cast<uint8_t>(reg[dst] - imm);
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // JNZ addr
  case 0x04: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    if (!zeroFlag)
      pc = addr;
    break;
  }
  // JMP addr
  case 0x05: {
    uint16_t addr = mem.read16(pc);
    pc = addr;
    break;
  }
  // CMP reg, reg
  case 0x06: {
    uint8_t dst = reg[mem.read8(pc++)];
    uint8_t dst2 = reg[mem.read8(pc++)];
    if (dst == dst2) {
      zeroFlag = true;
    } else {
      zeroFlag = false;
    }

    if (dst < dst2) {
      carryFlag = true;
    } else {
      carryFlag = false;
    }
    break;
  }
  // MOV_REG reg, reg
  case 0x07: {
    uint8_t dst = mem.read8(pc++);
    uint8_t target = mem.read8(pc++);
    reg[dst] = reg[target];
    break;
  }
  // MOV ADDR, IMM
  case 0x08: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t target = mem.read8(pc++);
    mem.write8(addr, target);
    break;
  }
  // MOV ADDR, REG
  case 0x09: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t target = mem.read8(pc++);
    mem.write8(addr, reg[target]);
    break;
  }
  // MOV REG, ADDR
  case 0x0A: {
    uint8_t dst = mem.read8(pc++);
    uint16_t target = mem.read16(pc);
    pc += 2;
    reg[dst] = mem.read8(target);
    break;
  }
  default: {
    cerr << "ERR: Illegal Opcode: 0x" << hex << (int)ir << dec << endl;
    cpuState = "down";
    isHalt = true;
    break;
  }
  }
}

void CPU::addInstruction(string instruction) {
  if (isHalt)
    return;

  vector<string> operands = getInstructionOperand(instruction);
  if (operands.empty())
    return;

  string mnemonic = operands[0];
  uint8_t opcode = 0;

  // 解析 Opcode
  if (mnemonic == "MOV") {
    if (operands.size() == 3) {
      if (regNameToIndex.find(operands[1]) != regNameToIndex.end()) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["MOV_REG"];
        } else if (checkMem(operands[2])) {
          opcode = codeMap["MOV_REG_ADDR"];
        } else {
          opcode = codeMap["MOV"];
        }
      } else if (checkMem(operands[1])) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["MOV_ADDR_REG"];
        } else if (checkMem(operands[2])) {
          cerr << "ERR: x86 doesn't support mem-to-mem!" << endl;
          return;
        } else {
          opcode = codeMap["MOV_ADDR"];
        }
      } else {
        cerr << "ERR: Dest cannot be immediate!" << endl;
        return;
      }
    } else {
      cerr << "ERR: MOV needs 2 operands!" << endl;
      return;
    }
  } else {
    opcode = codeMap[mnemonic];
  }

  // 写入 Opcode 到 loadAddress
  mem.write8(loadAddress, opcode);
  loadAddress++;

  // 写入操作数
  if (operands.size() > 1) {
    for (size_t i = 1; i < operands.size(); i++) {
      string opStr = operands[i];
      // 寄存器
      if (regNameToIndex.find(opStr) != regNameToIndex.end()) {
        mem.write8(loadAddress, regNameToIndex[opStr]);
        loadAddress += 1;
      }
      // 地址
      else if (checkMem(opStr)) {
        mem.write16(loadAddress, parseMemoryAddress(opStr));
        loadAddress += 2;
      }
      // 立即数/跳转地址
      else {
        uint16_t imm = static_cast<uint16_t>(stoi(opStr, nullptr, 0));
        if (opcode == 0x04 || opcode == 0x05) {
          mem.write16(loadAddress, imm);
          loadAddress += 2;
        } else {
          mem.write8(loadAddress, static_cast<uint8_t>(imm));
          loadAddress += 1;
        }
      }
    }
  }
}
