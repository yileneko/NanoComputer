#include "cpu.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

CPU *cpu = nullptr;

void initComputer() { cpu = new CPU(); }

void addInstruction(string instruction) { cpu->runInstruction(instruction); }

int main() {
  initComputer();

  addInstruction("HALT");

  cpu->step();

  cout << cpu->getCPUState();

  return 0;
}
