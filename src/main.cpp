#include "assembler.h"
#include "cpu.h"
#include "memory.h"
#include <cstddef>
#include <iostream>
#include <sstream>
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

  string programStr = "; 这是一个多行汇编程序\n"
                      "start:\n"
                      "    MOV AL, 5\n"
                      "    ADD AL, 3\n"
                      "    MOV [0x0100], AL\n"
                      "    HALT\n";

  vector<string> programLines = {};
  istringstream stream(programStr);
  string line;
  while (getline(stream, line)) {
    programLines.push_back(line);
  }

  ass->assembleProgram(programLines);

  cpu->setPC(0x0000);

  while (!cpu->getHaltState()) {
    cpu->step();
    cpu->printState();
  }

  cleanupComputer();
  return 0;
}
