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
  cmake ..
  cmake --build . --target test -j8
  ./test $@
  cd ..
fi
