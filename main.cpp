#include <format>
#include <fstream>
#include <iostream>
#include <map>

#include "llvm_patched/Support/CommandLine.h"

import fixed;
import fluid;
import meta;

using namespace llvm;

cl::opt<std::string> InputFilenameArg(cl::Positional, cl::Required, cl::desc("<filename>"));

cl::OptionCategory TypeSelectionCategory(
    "Type selection options",
    "\n  Available types:\n"
    "    FLOAT              - Built-in float\n"
    "    DOUBLE             - Built-in double\n"
    "    FIXED(N,K)         - Fixed point number represented as `intN_t` divided by 2^K\n"
    "    FIXED_FAST(N,K)    - Fixed point number represented as `int_fastN_t` divided by 2^K"
);

cl::opt<std::string> pTypeArg("p-type",
    cl::desc("Particle type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vTypeArg("v-type",
    cl::desc("Velocity type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vFlowTypeArg("v-flow-type",
    cl::desc("Velocity flow type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));

#define FLOAT float
#define DOUBLE double
#define FIXED(N, K) Fixed<N, K>
#define FAST_FIXED(N, K) FastFixed<N, K>

struct {
    std::map<std::string, size_t> type_ids;

    template <typename T, typename... Ts>
    constexpr void create_for() {
        static size_t id = 0;
        type_ids[TypeName<T>::as_string()] = id++;

        if constexpr (sizeof...(Ts) > 0) {
            create_for<Ts...>();
        }
    }

    constexpr size_t get_id(const std::string& name) {
        return type_ids[name];
    }
} types_mapping;


int main(int argc, char **argv) {
    cl::ParseCommandLineOptions(argc, argv);

    constexpr size_t N = 36, M = 84;
    char field[N][M + 1];
    std::ifstream input(InputFilenameArg);
    for (int i = 0; input.getline(field[i], M + 2) && i < N; ++i) {}

    types_mapping.create_for<TYPES>();
    const size_t pTypeId     = types_mapping.get_id(pTypeArg),
                 vTypeId     = types_mapping.get_id(vTypeArg),
                 vFlowTypeId = types_mapping.get_id(vFlowTypeArg);

    using types = meta::vector<TYPES>;
    meta::with_types<types>(fluid::run_simulation<N, M>(field),
        pTypeId, vTypeId, vFlowTypeId);
}