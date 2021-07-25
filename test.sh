<<<<<<< HEAD
if [ "$1" = "clean" ]; then
  rm test/*output -rf
else
  cd ./cmake-build-debug
=======
if [[ $1 == "clean" ]]; then
  rm -rf cmake-build-debug*
  cd test
  rm -rf *output
  cd ..
else
  if [[ -d ./cmake-build-debug ]]; then
    cd ./cmake-build-debug
  else
    mkdir ./cmake-build-debug
    cd ./cmake-build-debug
  fi
>>>>>>> 6a76acf1aeaee4d3422fd16346f235f6c477279a
  cmake ..
  cmake --build . --target ${1-"personal_test"}
  ./${1-"personal_test"}
  cd ..
fi
