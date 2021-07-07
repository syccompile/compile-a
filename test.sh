if [ "$1" = "clean" ]; then
  rm test/*output -rf
else
  cd ./cmake-build-debug
  cmake ..
  cmake --build . --target ${1-"personal_test"}
  ./${1-"personal_test"}
  cd ..
fi
