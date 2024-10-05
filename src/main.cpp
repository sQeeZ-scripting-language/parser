#include "main.hpp"

int main(int argc, char* argv[]) {
  bool devMode = false;
  std::string filename;

  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " <filename>.sqz [--dev]" << std::endl;
    return 1;
  }

  if (argc == 2) {
    filename = argv[1];
  } else if (argc == 3) {
    if (std::string(argv[2]) == "--dev") {
      devMode = true;
      filename = argv[1];
    } else if (std::string(argv[1]) == "--dev") {
      devMode = true;
      filename = argv[2];
    } else {
      std::cerr << "Error: Unrecognized argument: " << argv[2] << std::endl;
      return 1;
    }
  }

  std::size_t index = filename.find_last_of(".");
  if (index == std::string::npos || filename.substr(index + 1) != "sqz") {
    std::cerr << "Error: File must have a .sqz extension" << std::endl;
    return 1;
  }

  std::ifstream file(filename);
  if (!file) {
    std::cerr << "Cannot read file: " << filename << std::endl;
    return 1;
  }

  std::string code;
  std::string line;
  while (std::getline(file, line)) {
    code += line + "\n";
  }

  if (devMode) {
    std::cout << "Developer mode activated!\n" << std::endl;
  }

  return 0;
}