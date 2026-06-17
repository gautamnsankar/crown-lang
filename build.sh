set -e
g++ -std=c++20 -O0 -I include src/index.cpp -o compiler $(llvm-config --system-libs --libs core support)

./compiler main.cw -o main.o
g++ -std=c++20 -O0 -c runtime/stdlib.cpp -o stdlib.o
g++ -no-pie main.o stdlib.o -o program

./program