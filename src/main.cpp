#include "cpu.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

CPU *cpu = nullptr;

void initComputer() { cpu = new CPU(); }

void addInstruction(string instruction) { cpu->addInstruction(instruction); }

int main() {
  initComputer();

  addInstruction("MOV AL, 0");
  addInstruction("MOV BL, 3");
  addInstruction("ADD AL, BL");
  addInstruction("SUB BL, 1");
  addInstruction("JNZ 2");
  addInstruction("HALT");

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  return 0;
}
