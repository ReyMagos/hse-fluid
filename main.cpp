#include <fstream>
#include <iostream>

#include "llvm_patched/Support/CommandLine.h"

import fixed;
import fluid;

using namespace llvm;

cl::opt<std::string> InputFilenameArg(cl::Positional, cl::Required, cl::desc("<filename>"));

cl::OptionCategory TypeSelectionCategory(
    "Type selection options",
    "\n  Available types:\n"
    "    FLOAT              - Built-in float\n"
    "    DOUBLE             - Built-in double\n"
    "    FIXED(N, K)        - Fixed point number represented as `intN_t` divided by 2^K\n"
    "    FIXED_FAST(N, K)   - Fixed point number represented as `int_fastN_t` divided by 2^K"
);

cl::opt<std::string> pTypeArg("p-type",
    cl::desc("Particle type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vTypeArg("v-type",
    cl::desc("Velocity type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vFlowTypeArg("v-flow-type",
    cl::desc("Velocity flow type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));


int main(int argc, char **argv) {
    cl::ParseCommandLineOptions(argc, argv);

    constexpr size_t N = 36, M = 84;

    char field[N][M + 1];
    std::ifstream input(InputFilenameArg);

    for (int i = 0; input.getline(field[i], M + 2) && i < N; ++i) {}

    std::ofstream file_output;
    file_output.open("output.txt", std::ios::trunc);

    fluid::Simulation<Fixed<int32_t, 16>, Fixed<int32_t, 16>, Fixed<int32_t, 16>, N, M> simulation(field);
    simulation.render(file_output);

    file_output.close();
}