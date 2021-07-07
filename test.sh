cd ./cmake-build-debug
cmake ..
cmake --build . --target ${1-"personal_test"}
./${1-"personal_test"}
cd ..
