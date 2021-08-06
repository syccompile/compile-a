#ifndef SYSYCOMPILER_TEST_TEST_HPP_
#define SYSYCOMPILER_TEST_TEST_HPP_

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
namespace fs = std::filesystem;

fs::path compiler{"./compiler"};
std::string link_flags = " -g -static ";
inline std::vector<std::string> passed_cases, ignored_cases, compile_error_cases, failed_cases;
const std::string normal = "\033[0m";
const std::string black = "\033[0;30m";
const std::string red = "\033[0;31m";
const std::string green = "\033[0;32m";
const std::string yellow = "\033[0;33m";
const std::string blue = "\033[0;34m";
const std::string white = "\033[0;37m";

bool check_same(const fs::path &file1, const fs::path &file2) {
  // use diff to compare
  auto cmp_command = "diff " + file1.string() + " " + file2.string() + " -b -B";
  int exit_code = std::system(cmp_command.c_str());
  return exit_code == 0;
}

void test(const fs::path &testFile, const fs::path &outputDir) {
  auto outputIRFile = outputDir / (testFile.stem().string() + ".ir");
  auto outputASMFile = outputDir / (testFile.stem().string() + ".s");
  auto outputResultFile = outputDir / (testFile.stem().string() + ".out");
  auto outputExeFile = outputDir / testFile.stem();
  auto testDir = testFile.parent_path();
  auto expectedResultFile = testDir / (testFile.stem().string() + ".out");
  auto inputFile = testDir / (testFile.stem().string() + ".in");

  auto compile_command = compiler.string() + " " + testFile.string() + " -r " +
      outputIRFile.string() + " -o " + outputASMFile.string() + " -O2";
  auto link_command = "gcc -march=armv7 " + outputASMFile.string() +
      " -L. -lsysy -o " + outputExeFile.string()
      + link_flags;
  auto run_command = outputExeFile.string() + " > " + outputResultFile.string();

  const std::string &testFilename = testFile.filename().string();
  std::cout << blue << "testing " << testFilename << ": " << normal << std::endl;
  int exit_code;

  /* compile */
  exit_code = std::system(compile_command.c_str());
  if (exit_code != 0) {
    std::cout << red << "\tfail to compile " << testFilename << normal << std::endl << std::endl;
    compile_error_cases.push_back(testFile.filename());
    return;
  } else {
    std::cout << green << "\tcompile " << testFilename << " successfully" << normal << std::endl;
  }

//  if (fs::exists(expectedResultFile)) { // link and run
//    /* link */
//    exit_code = std::system(link_command.c_str());
//    if (exit_code != 0) {
//      std::cout << red << "\tfail to link " << testFilename << normal << std::endl << std::endl;
//      failed_cases.push_back(testFile.filename());
//      return;
//    } else {
//      std::cout << green << "\tlink successfully" << testFilename << normal << std::endl;
//    }
//
//    /* run */
//    if (fs::exists(inputFile)) { // has input files
//      run_command += (" < " + inputFile.string());
//    }
//    exit_code = std::system(run_command.c_str());
//    std::ofstream ofs(outputResultFile, std::ios::app);
//    ofs << std::endl << (exit_code >> 8) << std::endl;
//    ofs.close();
//
//    /* check */
//    if (check_same(outputResultFile, expectedResultFile)) {
//      std::cout << green << "\tpassed" << normal << std::endl;
//      passed_cases.push_back(testFilename);
//    } else {
//      std::cout << red << "\tresult differs from expected, failed" << normal << std::endl;
//      failed_cases.push_back(testFilename);
//    }
//
//  } else {  // just compile
//    ignored_cases.push_back(testFilename);
//    std::cout << yellow << "\tcouldn't find " << expectedResultFile.filename().string() << ", ignored" << normal
//              << std::endl;
//  }
  std::cout << std::endl;
}

void printResult() {
  if (failed_cases.empty()) {
    std::cout << green << "All tests passed!" << normal << std::endl;
  } else {
    std::cout << red << "Compile Error cases: " << normal << std::endl;
    std::copy(compile_error_cases.begin(),
              compile_error_cases.end(),
              std::ostream_iterator<std::string>(std::cout, "\n"));
    std::cout << red << "Failed cases: " << normal << std::endl;
    std::copy(failed_cases.begin(), failed_cases.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
//    std::cout << std::endl << green << "Passed cases: " << normal << std::endl;
//    std::copy(passed_cases.begin(), passed_cases.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
//    std::cout << std::endl << yellow << "Ignored cases: " << normal << std::endl;
//    std::copy(ignored_cases.begin(), ignored_cases.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
    std::cout << std::endl;
  }
}

#endif //SYSYCOMPILER_TEST_TEST_HPP_
