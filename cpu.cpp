#include "cpu.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

CPU::CPU() {
  instructionStream.clear();
  pc = 0;
  ir = "";

  cpuState = "running";
  isHalt = false;
};

string CPU::getCPUState() { return cpuState; }

void CPU::step() {
  if (isHalt) {
    cpuState = "down";
    return;
  }
  cpuState = "running";
  if (instructionStream.empty()) {
    return;
  }
  if (pc >= instructionStream.size()) {
    cerr << "PC out of range: missing HALT" << endl;
    return;
  }

  ir = instructionStream[pc];

  pc++;

  runInstruction(ir);
}

void CPU::addInstruction(string instruction) {
  if (isHalt)
    return;
  instructionStream.push_back(instruction);
}

void CPU::runInstruction(string instruction) {
  if (instruction == "HALT") {
    isHalt = true;
  }
}
