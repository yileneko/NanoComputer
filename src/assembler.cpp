#include "assembler.h"
#include "memory.h"
#include <filesystem>
#include <iostream>
#include <stdint.h>
#include <vector>

using namespace std;

Assembler::Assembler(Memory &mem) : mem(mem) {};

uint16_t parseMemoryAddress(const std::string &token) {
  if (token.length() < 2 || token.front() != '[' || token.back() != ']') {
    cerr << "ERR: Invalid memory format! " << "memory: " << token << endl;
  }

  std::string innerStr = token.substr(1, token.length() - 2);

  try {
    uint32_t value = std::stoul(innerStr, nullptr, 0);

    if (value > 0xFFFF) {
      cerr << "ERR: Address out of 16-bit range! " << "address: " << innerStr
           << endl;
    }

    return static_cast<uint16_t>(value);
  } catch (const std::exception &e) {
    cerr << "ERR: Failed to parse memory address! " << "address: " << innerStr
         << endl;
  }

  return 0;
}

static std::string toHexString(uint8_t v) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(v);
  return oss.str();
}

bool checkMem(string str) {
  if (str[0] == '[' && str[str.size() - 1] == ']') {
    return true;
  }

  return false;
}

string getInstructionType(string instruction) {
  auto spacePos = instruction.find_first_of(" \t,");
  if (spacePos == string::npos)
    return instruction;
  return instruction.substr(0, spacePos);
}

vector<string> getInstructionOperand(string instruction) {
  vector<string> ret = {};
  size_t pos = 0;
  while (1) {
    auto start = instruction.find_first_not_of(" \t,", pos);
    if (start == string::npos)
      break;

    auto end = instruction.find_first_of(" \t,", start);
    if (end == string::npos) {
      ret.push_back(instruction.substr(start));
      break;
    }
    ret.push_back(instruction.substr(start, end - start));
    pos = end + 1;
  }
  return ret;
}

string cleanLine(string line) {
  size_t commentPos = line.find(';');
  string clean =
      (commentPos == string::npos) ? line : line.substr(0, commentPos);

  size_t firstSpacePos = clean.find_first_not_of(" \t\r\n");
  if (firstSpacePos == string::npos)
    return "";

  size_t lastSPacePos = clean.find_last_not_of(" \t\r\n");

  return clean.substr(firstSpacePos, lastSPacePos - firstSpacePos + 1);
}

void Assembler::assembleProgram(const vector<string> &program) {
  labelMap.clear();
  loadAddress = 0;

  for (string rawLine : program) {
    string line = cleanLine(rawLine);

    if (line.empty())
      continue;

    size_t colonPos = line.find(':');
    if (colonPos != string::npos) {
      string labelName = line.substr(0, colonPos);
      labelName.erase(0, labelName.find_first_not_of(" \t"));
      labelName.erase(labelName.find_last_not_of(" \t") + 1);
      labelMap[labelName] = loadAddress;

      string restOfLine = line.substr(colonPos + 1);
      if (restOfLine.find_first_not_of(" \t") == string::npos)
        continue;

      line = restOfLine;
    }

    assemble(line, true);
  }

  loadAddress = 0;
  for (string rawLine : program) {
    string line = cleanLine(rawLine);
    if (line.empty())
      continue;

    size_t colonPos = line.find(':');
    if (colonPos != string::npos) {
      string restOfLine = line.substr(colonPos + 1);
      if (restOfLine.find_first_not_of(" \t") == string::npos)
        continue;
      line = restOfLine;
    }

    assemble(line, false);
  }
}

void Assembler::assemble(string instruction, bool isDryRun) {
  vector<string> operands = getInstructionOperand(instruction);
  if (operands.empty())
    return;

  string mnemonic = operands[0];
  uint8_t opcode = 0;
  // MOV及其变体
  if (mnemonic == "MOV") {
    if (operands.size() == 3) {
      if (regNameToIndex.find(operands[1]) != regNameToIndex.end()) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["MOV_REG"];
        } else if (checkMem(operands[2])) {
          opcode = codeMap["MOV_REG_ADDR"];
        } else {
          opcode = codeMap["MOV"];
        }
      } else if (checkMem(operands[1])) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["MOV_ADDR_REG"];
        } else if (checkMem(operands[2])) {
          cerr << "ERR: x86 doesn't support mem-to-mem!" << endl;
          return;
        } else {
          opcode = codeMap["MOV_ADDR"];
        }
      } else {
        cerr << "ERR: Dest cannot be immediate!" << endl;
        return;
      }
    } else {
      cerr << "ERR: MOV needs 2 operands!" << endl;
      return;
    }
  }
  // ADD及其变体
  else if (mnemonic == "ADD") {
    if (operands.size() == 3) {
      if (regNameToIndex.find(operands[1]) != regNameToIndex.end()) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["ADD_REG"];
        } else if (checkMem(operands[2])) {
          opcode = codeMap["ADD_REG_ADDR"];
        } else {
          opcode = codeMap["ADD"];
        }
      } else if (checkMem(operands[1])) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["ADD_ADDR_REG"];
        } else if (checkMem(operands[2])) {
          cerr << "ERR: x86 doesn't support mem-to-mem!" << endl;
          return;
        } else {
          opcode = codeMap["ADD_ADDR"];
        }
      } else {
        cerr << "ERR: Dest cannot be immediate!" << endl;
        return;
      }
    } else {
      cerr << "ERR: ADD needs 2 operands!" << endl;
      return;
    }
  } else if (mnemonic == "SUB") {
    if (operands.size() == 3) {
      if (regNameToIndex.find(operands[1]) != regNameToIndex.end()) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["SUB_REG"];
        } else if (checkMem(operands[2])) {
          opcode = codeMap["SUB_REG_ADDR"];
        } else {
          opcode = codeMap["SUB"];
        }
      } else if (checkMem(operands[1])) {
        if (regNameToIndex.find(operands[2]) != regNameToIndex.end()) {
          opcode = codeMap["SUB_ADDR_REG"];
        } else if (checkMem(operands[2])) {
          cerr << "ERR: x86 doesn't support mem-to-mem!" << endl;
          return;
        } else {
          opcode = codeMap["SUB_ADDR"];
        }
      } else {
        cerr << "ERR: Dest cannot be immediate!" << endl;
        return;
      }
    } else {
      cerr << "ERR: SUB needs 2 operands!" << endl;
      return;
    }
  } else {
    if (codeMap.find(mnemonic) != codeMap.end())
      opcode = codeMap[mnemonic];
    else {
      cerr << "ERR: unknown opcode! " << "instruction: " << instruction << endl;
      return;
    }
  }

  // 写入 Opcode 到 loadAddress
  if (!isDryRun) {
    mem.write8(loadAddress, opcode);
  }
  loadAddress++;

  // 写入操作数
  if (operands.size() > 1) {
    for (size_t i = 1; i < operands.size(); i++) {
      string opStr = operands[i];
      // 寄存器
      if (regNameToIndex.find(opStr) != regNameToIndex.end()) {
        if (!isDryRun)
          mem.write8(loadAddress, regNameToIndex[opStr]);
        loadAddress += 1;
      }
      // 地址
      else if (checkMem(opStr)) {
        if (!isDryRun)
          mem.write16(loadAddress, parseMemoryAddress(opStr));
        loadAddress += 2;
      }
      // 立即数/跳转地址
      else {
        uint16_t imm = 0;

        if (labelMap.find(opStr) != labelMap.end()) {
          imm = labelMap[opStr];
        } else {
          try {
            imm = static_cast<uint16_t>(stoi(opStr, nullptr, 0));
          } catch (...) {
            cerr << "ERR: Invalid number of undefined label: " << opStr << endl;
            return;
          }
        }

        if (opcode == 0x04 || opcode == 0x05) {
          if (!isDryRun)
            mem.write16(loadAddress, imm);
          loadAddress += 2;
        } else {
          if (!isDryRun)
            mem.write8(loadAddress, static_cast<uint8_t>(imm));
          loadAddress += 1;
        }
      }
    }
  }
}
