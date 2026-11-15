#include "cpu.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

std::string toHexString(uint8_t v) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(v);
  return oss.str();
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

CPU::CPU() {
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
      cerr << "wrong operands! type: " << ins.opcode
           << " the number of operands: " << operands.size() << endl;
      return;
    }
  }
  if (operands.size() > 1) {
    ins.operands = {};
    for (int i = 1; i < operands.size(); i++) {
      ins.operands.push_back(static_cast<uint8_t>(stoi(operands[i])));
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
    uint8_t imm = static_cast<uint8_t>(ins.operands[1]);
    reg[ins.operands[0]] = imm;
    break;
  }
  };
}
