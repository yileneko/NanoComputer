#include "assembler.h"
#include "cpu.h"
#include "memory.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

Memory *mem = nullptr;
Assembler *ass = nullptr;
CPU *cpu = nullptr;

void initComputer() {
  mem = new Memory();
  ass = new Assembler(*mem);
  cpu = new CPU(*mem);
}

void cleanupComputer() {
  delete mem;
  delete cpu;
  mem = nullptr;
  cpu = nullptr;
}

int main() {
  initComputer();

  ass->assemble("MOV AL, 5");
  ass->assemble("MOV [0x0100], AL");
  ass->assemble("MOV BL, [0x0100]");
  ass->assemble("CMP AL, BL");
  ass->assemble("JNZ 0x001E");
  ass->assemble("ADD AL, 2");
  ass->assemble("SUB AL, 1");
  ass->assemble("MOV [0x0200], AL");
  ass->assemble("JMP 0x0023");
  ass->assemble("MOV [0x0200], 99");
  ass->assemble("HALT");
  ass->assemble("HALT");

  cpu->setPC(0x0000);

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  cleanupComputer();
  return 0;
}
