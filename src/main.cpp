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

  addInstruction("MOV AL, 0");
  addInstruction("MOV BL, 3");
  addInstruction("ADD AL, BL");
  addInstruction("SUB BL, 1");
  addInstruction("JNZ 2");
  addInstruction("MOV [0x0100], AL");
  addInstruction("HALT");

  cout << "Start Running" << endl;
  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  cout << "\nTask Ended" << endl;
  cout << "[0x0100]: " << static_cast<int>(mem->read8(0x0100)) << endl;

  cleanupComputer();

  return 0;
}
