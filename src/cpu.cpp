#include "cpu.h"
#include "memory.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
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
  instructionStream.clear();
  pc = 0;
  reg.clear();
  for (int i = 0; i < 8; i++) {
    reg.push_back(0);
  }

  cpuState = "running";
  isHalt = false;
};

bool CPU::getHaltState() { return isHalt; }

string CPU::getCPUState() { return cpuState; }

uint8_t CPU::readOperandValue(const Operand &op) {
  switch (op.type) {
  case OperandType::REG: {
    return reg[op.value];
  }
  case OperandType::IMM: {
    return op.value;
  }
  case OperandType::MEM: {
    return mem.read8(op.mem);
  }
  }
  return 0;
}

void CPU::writeOperandValue(const Operand &op, uint8_t value) {
  switch (op.type) {
  case OperandType::REG: {
    reg[op.value] = value;
    break;
  }
  case OperandType::IMM: {
    cerr << "ERR: Cannot write to immediate value!" << endl;
    break;
  }
  case OperandType::MEM: {
    mem.write8(op.mem, value);
    break;
  }
  }
}

void CPU::printState() {
  cout << "PC: " << pc << " " << "IR: " << toHexString(ir) << " "
       << "cpuState: " << cpuState << endl
       << "register: ";
  for (int i = 0; i < reg.size(); i++) {
    cout << toHexString(reg[i]) << " ";
  }
  cout << endl;
}

void CPU::step() {
  if (isHalt)
    return;
  cpuState = "running";
  if (instructionStream.empty()) {
    return;
  }
  if (pc >= instructionStream.size()) {
    cerr << "PC out of range: missing HALT" << endl;
    isHalt = true;
    cpuState = "down";
    return;
  }

  ir = instructionStream[pc].opcode;

  pc++;

  runInstruction(instructionStream[pc - 1]);
}

void CPU::addInstruction(string instruction) {
  if (isHalt)
    return;
  Instruction ins;
  ins.opcode = codeMap[getInstructionType(instruction)];
  vector<string> operands = getInstructionOperand(instruction);
  if (ins.opcode == 0x01) {
    if (operands.size() != 3) {
      cerr << "ERR: wrong operands! type! " << toHexString(ins.opcode)
           << " the number of operands: " << operands.size() << endl;
      return;
    }
  }
  if (operands.size() > 1) {
    ins.operands = {};
    for (int i = 1; i < operands.size(); i++) {
      Operand op;
      if (regNameToIndex.find(operands[i]) != regNameToIndex.end()) {
        op.type = OperandType::REG;
        op.value = regNameToIndex[operands[i]];
        op.mem = 0;
      } else if (checkMem(operands[i])) {
        op.type = OperandType::MEM;
        op.value = 0;
        op.mem = parseMemoryAddress(operands[i]);
      } else {
        op.type = OperandType::IMM;
        op.value = static_cast<uint8_t>(stoi(operands[i]));
        op.mem = 0;
      }
      ins.operands.push_back(op);
    }
  }

  instructionStream.push_back(ins);
}

void CPU::runInstruction(Instruction ins) {
  uint8_t code = ins.opcode;

  switch (code) {
  case 0xFF: {
    cpuState = "down";
    isHalt = true;
    break;
  }
  case 0x01: {
    if (ins.operands.size() != 2) {
      cerr << "ERR: failed to run a instruction! "
           << "instruction: " << toHexString(ins.opcode) << endl;
      break;
    }

    uint8_t newVal = readOperandValue(ins.operands[1]);
    writeOperandValue(ins.operands[0], newVal);

    break;
  }
  case 0x02: {
    if (ins.operands.size() != 2) {
      cerr << "ERR: failed to run a instruction! "
           << "instruction: " << toHexString(ins.opcode) << endl;
      break;
    }

    uint8_t newVal =
        readOperandValue(ins.operands[0]) + readOperandValue(ins.operands[1]);

    writeOperandValue(ins.operands[0], newVal);

    zeroFlag = (newVal == 0);

    break;
  }
  case 0x03: {
    if (ins.operands.size() != 2) {
      cerr << "ERR: failed to run a instruction! "
           << "instruction: " << toHexString(ins.opcode) << endl;
      break;
    }

    uint8_t newVal =
        readOperandValue(ins.operands[0]) - readOperandValue(ins.operands[1]);

    writeOperandValue(ins.operands[0], newVal);

    zeroFlag = (newVal == 0);

    break;
  }
  case 0x04: {
    if (ins.operands.size() != 1) {
      cerr << "ERR: failed to run a instruction! "
           << "instruction: " << toHexString(ins.opcode) << endl;
      break;
    }
    if (!zeroFlag) {
      if (ins.operands[0].type == OperandType::REG ||
          ins.operands[0].type == OperandType::IMM) {
        pc = readOperandValue(ins.operands[0]);
      } else {
        pc = ins.operands[0].mem;
      }
    }
    break;
  }
  case 0x05: {
    if (ins.operands.size() != 1)
      break;

    if (ins.operands[0].type == OperandType::REG) {
      pc = reg[ins.operands[0].value];
    } else if (ins.operands[0].type == OperandType::IMM) {
      pc = ins.operands[0].value;
    } else {
      pc = mem.read16(ins.operands[0].mem);
    }
  }
  };
}
