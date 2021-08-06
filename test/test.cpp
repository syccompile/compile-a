#include "test.hpp"

void get_files(std::vector<fs::path> &test_files, const std::string& str) {
  fs::path root_dir = "../test";
  for (const auto &f : fs::recursive_directory_iterator(root_dir)) {
    if (f.is_regular_file()) {
      auto f_name = f.path().filename().string();
      if (f_name.find(str) != std::string::npos) {
        test_files.push_back(f);
      }
    } else if (f.is_directory()) {
      auto f_name = f.path().filename().string();
      if (f_name.find(str) != std::string::npos) {
        for (const auto &t : fs::recursive_directory_iterator(f)) {
          if (t.is_regular_file()) {
            test_files.push_back(t);
          }
        }
      }
    } // ignore else
  }
}


int main(int argc, char *argv[]) {
  if (!fs::is_regular_file(compiler)) {
    std::cerr << "compiler not found" << std::endl;
    exit(EXIT_FAILURE);
  }

  fs::path outputDir{"../output"};
  fs::create_directory(outputDir);

  std::vector<fs::path> test_files;
  if (argc == 0) {
    std::cerr << "usage: ./test <test_files>" << std::endl;
  }
  for (int i = 1; i < argc; i++) {
    get_files(test_files, argv[i]);
  }
  for (auto iter = test_files.begin(); iter != test_files.end();) {
    if ((*iter).extension() != ".sy") {
      iter = test_files.erase(iter);
    } else {
      ++iter;
    }
  }
  std::cout << "testing " << test_files.size() << " files." << '\n';
  for (const auto &test_file : test_files) {
    test(test_file, outputDir);
  }
  printResult();
  return 0;
}
