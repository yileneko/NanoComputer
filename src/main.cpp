#include "cpu.h"
#include "memory.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

Memory *mem = nullptr;
CPU *cpu = nullptr;

void initComputer() {
  mem = new Memory();
  cpu = new CPU(*mem);
}

void cleanupComputer() {
  delete mem;
  delete cpu;
  mem = nullptr;
  cpu = nullptr;
}

void addInstruction(string instruction) { cpu->addInstruction(instruction); }

int main() {
  initComputer();

  cpu->addInstruction("MOV AL, 1");
  cpu->addInstruction("JMP 3");
  cpu->addInstruction("MOV AL, 99");
  cpu->addInstruction("MOV AL, 2");
  cpu->addInstruction("HALT");

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  cleanupComputer();

  return 0;
}
