#include "assembler.h"
#include "cpu.h"
#include "memory.h"
#include <cstddef>
#include <fstream>
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

  string asmFilePath;
  cout << "Please enter the path of your asm file: ";
  cin >> asmFilePath;

  string programStr;
  ifstream file(asmFilePath);
  if (!file.is_open()) {
    cerr << "ERR: failed to open file: " << asmFilePath << endl;
    return -1;
  }
  stringstream buffer;
  buffer << file.rdbuf();
  programStr = buffer.str();

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
