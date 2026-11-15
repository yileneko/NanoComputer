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

  addInstruction("MOV 0,8");
  addInstruction("HALT");

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  return 0;
}
