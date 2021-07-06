#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <future>
namespace fs = std::filesystem;

// assume executable test and compiler stored in compiler-a/cmake-build-debug
fs::path compiler{"./compiler"};
fs::path inputDir{"../test/input"};
fs::path outputDir{"../test/output"};

void compile(const fs::path &inFile, const fs::path &outDir) {
  auto outIRFile = outDir / (inFile.stem().string() + ".ir");
  auto outASMFile = outDir / (inFile.stem().string() + ".asm");
  auto command = compiler.string() + " " + inFile.string() + " " +
      outIRFile.string() + " " + outASMFile.string();
  std::system(command.c_str());
}

int main() {
  if (!fs::is_regular_file(compiler)) {
    std::cerr << "compiler not found" << std::endl;
    exit(EXIT_FAILURE);
  }
  fs::create_directory(inputDir);
  fs::create_directory(outputDir);

  std::vector<std::future<void>> jobs;
  fs::directory_iterator inputIter{inputDir};
  for (const fs::path &file : inputIter) {
    jobs.push_back(std::async(std::launch::async, compile, file, outputDir));
  }
  for (auto& job : jobs) {
    job.wait();
  }
  return 0;
}