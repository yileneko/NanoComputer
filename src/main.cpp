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

  addInstruction("MOV AL, 8");
  addInstruction("ADD BL, 3");
  addInstruction("ADD AH, AL");
  addInstruction("SUB AH, 1");
  addInstruction("SUB AH, BL");
  addInstruction("HALT");

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  return 0;
}
