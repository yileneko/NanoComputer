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

static std::string toHexString(uint8_t v) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(v);
  return oss.str();
}

CPU::CPU(Memory &mem) : mem(mem) {
  pc = 0;
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
  // ADD REG, REG
  case 0x0B: {
    uint8_t dst = mem.read8(pc++);
    uint8_t target = mem.read8(pc++);
    reg[dst] = static_cast<uint8_t>(reg[dst] + reg[target]);
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // ADD  ADDR, IMM
  case 0x0C: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t imm = mem.read8(pc++);
    uint8_t n = mem.read8(addr) + imm;
    mem.write8(addr, n);
    zeroFlag = (n == 0);
    break;
  }
  // ADD ADDR, REG
  case 0x0D: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t target = mem.read8(pc++);
    uint8_t n = mem.read8(addr) + reg[target];
    mem.write8(addr, n);
    zeroFlag = (n == 0);
    break;
  }
  // ADD REG, ADDR
  case 0x0E: {
    uint8_t dst = mem.read8(pc++);
    uint16_t target = mem.read16(pc);
    pc += 2;
    reg[dst] = static_cast<uint8_t>(reg[dst] + mem.read8(target));
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // SUB REG, REG
  case 0x0F: {
    uint8_t dst = mem.read8(pc++);
    uint8_t target = mem.read8(pc++);
    reg[dst] = static_cast<uint8_t>(reg[dst] - reg[target]);
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // SUB ADDR, IMM
  case 0x10: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t imm = mem.read8(pc++);
    uint8_t n = mem.read8(addr) - imm;
    mem.write8(addr, n);
    zeroFlag = (n == 0);
    break;
  }
  // SUB ADDR, REG
  case 0x11: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    uint8_t target = mem.read8(pc++);
    uint8_t n = mem.read8(addr) - reg[target];
    mem.write8(addr, n);
    zeroFlag = (n == 0);
    break;
  }
  // SUB REG, ADDR
  case 0x12: {
    uint8_t dst = mem.read8(pc++);
    uint16_t target = mem.read16(pc);
    pc += 2;
    reg[dst] = static_cast<uint8_t>(reg[dst] - mem.read8(target));
    zeroFlag = (reg[dst] == 0);
    break;
  }
  // JZ addr
  case 0x13: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    if (zeroFlag) {
      pc = addr;
    }
    break;
  }
  // JC addr
  case 0x14: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    if (carryFlag) {
      pc = addr;
    }
    break;
  }
  // JNC addr
  case 0x15: {
    uint16_t addr = mem.read16(pc);
    pc += 2;
    if (!carryFlag) {
      pc = addr;
    }
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
