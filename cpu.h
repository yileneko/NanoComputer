#pragma once

#include <string>
#include <vector>

using namespace std;

class CPU {
public:
  CPU();

  string getCPUState();
  void printState();
  void step();
  void addInstruction(string instruction);
  void runInstruction(string instruction);

private:
  string cpuState;
  bool isHalt;
  vector<string> instructionStream;
  size_t pc;
  string ir = "";
};
