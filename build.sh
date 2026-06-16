set -e
g++ -std=c++20 -O0 -I include src/index.cpp -o main $(llvm-config --system-libs --libs core support)
./main