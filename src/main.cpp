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

  cpu->addInstruction("MOV AL, 5");
  cpu->addInstruction("MOV [0x0100], AL");
  cpu->addInstruction("MOV BL, [0x0100]");
  cpu->addInstruction("CMP AL, BL");
  cpu->addInstruction("JNZ 0x001E");
  cpu->addInstruction("ADD AL, 2");
  cpu->addInstruction("SUB AL, 1");
  cpu->addInstruction("MOV [0x0200], AL");
  cpu->addInstruction("JMP 0x0023");
  cpu->addInstruction("MOV [0x0200], 99");
  cpu->addInstruction("HALT");
  cpu->addInstruction("HALT");

  cpu->setPC(0x0000);

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  cout << "内存 0x0100 的值: " << (int)mem->read8(0x0100) << " (预期: 5)"
       << endl;
  cout << "内存 0x0200 的值: " << (int)mem->read8(0x0200) << " (预期: 6)"
       << endl;

  cleanupComputer();

  return 0;
}
