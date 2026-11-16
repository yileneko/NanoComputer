#include "assembler.h"
#include "cpu.h"
#include "memory.h"
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

Memory *mem = nullptr;
Assembler *ass = nullptr;
CPU *cpu = nullptr;

static std::string toHexString(uint8_t v) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(v);
  return oss.str();
}

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

  cout << "内存机器码 Dump(前 64 字节) " << endl;
  for (int i = 0; i < 64; i++) {
    cout << hex << setw(2) << setfill('0') << (int)mem->read8(i) << " ";
    if ((i + 1) % 16 == 0)
      cout << endl;
  }
  cout << dec << "\n数据区 0x00F0~0x00FF (十进制): ";
  for (int i = 0x00F0; i <= 0x00FF; i++)
    cout << (int)mem->read8(i) << " ";
  cout << endl;

  cleanupComputer();

  return 0;
}
