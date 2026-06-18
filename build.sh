set -e
g++ -std=c++20 -O0 -I include src/index.cpp -o compiler $(llvm-config --system-libs --libs core support)

./compiler main.cw -o main.o
g++ -std=c++20 -O0 -c runtime/runtime.cpp -o runtime.o
g++ -no-pie main.o runtime.o -o program

./program