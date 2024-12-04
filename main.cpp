#include <fstream>
#include <iostream>

#include "llvm/Support/CommandLine.h"

import fluid;

using namespace llvm;

cl::opt<std::string> InputFilename("i", cl::desc("Specify input file"), cl::value_desc("filename"));

int main(int argc, char **argv) {
    cl::ParseCommandLineOptions(argc, argv);

    constexpr size_t N = 36, M = 84;

    char field[N][M + 1];
    std::ifstream input(InputFilename);

    for (int i = 0; input.getline(field[i], M + 2) && i < N; ++i) {}

    FluidSimulation<N, M> flow(field);
    flow.render();
}