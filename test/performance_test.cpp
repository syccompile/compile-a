#include "test.hpp"

fs::path inputDir{"../test/performance_test"};
fs::path outputDir{"../test/performance_output"};

int main() {
  if (!fs::is_regular_file(compiler)) {
    std::cerr << "compiler not found" << std::endl;
    exit(EXIT_FAILURE);
  }
  fs::create_directory(outputDir);

  fs::directory_iterator inputIter{inputDir};
  for (const fs::path &file : inputIter) {
    auto extention = file.extension().string();
    if (extention == ".sy") {
      test(file, outputDir);
    }
  }
  printResult();
  return 0;
}
