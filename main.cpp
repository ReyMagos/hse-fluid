#include <format>
#include <fstream>
#include <iostream>
#include <map>

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
    "    FIXED(N,K)         - Fixed point number represented as `intN_t` divided by 2^K\n"
    "    FIXED_FAST(N,K)    - Fixed point number represented as `int_fastN_t` divided by 2^K"
);

cl::opt<std::string> pTypeArg("p-type",
    cl::desc("Particle type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vTypeArg("v-type",
    cl::desc("Velocity type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));
cl::opt<std::string> vFlowTypeArg("v-flow-type",
    cl::desc("Velocity flow type"), cl::value_desc("type"), cl::cat(TypeSelectionCategory));

namespace meta {
    template <typename... T>
    struct vector {};


    template <typename V, size_t N>
    struct at_impl;

    template <template <typename...> typename V, typename T, typename... Ts>
    struct at_impl<V<T, Ts...>, 0> {
        using type = T;
    };

    template <template <typename...> typename V, typename T, typename... Ts, size_t N>
    struct at_impl<V<T, Ts...>, N> {
        static_assert(N <= sizeof...(Ts), "Invalid index");
        using type = typename at_impl<V<Ts...>, N - 1>::type;
    };

    template <typename V, size_t N>
    using at = typename at_impl<V, N>::type;


    template <size_t D, size_t K, typename V, typename... Res>
    struct with_types_impl {
        template <typename T, typename... IDs>
        static constexpr void execute(const T& fn, size_t id, IDs... ids) {
            if (K == id) {
                with_types_impl<D - 1, 0, V, Res..., at<V, K>>::execute(fn, ids...);
            } else {
                with_types_impl<D, K + 1, V, Res...>::execute(fn, id, ids...);
            }
        }
    };

    template <size_t D, template <typename...> typename V, typename... Ts, typename... Res>
    struct with_types_impl<D, sizeof...(Ts), V<Ts...>, Res...> {
        template <typename T, typename... IDs>
        static constexpr void execute(const T& fn, size_t id, IDs... ids) {
            throw std::invalid_argument("Invalid id");
        }
    };

    template <typename V, typename... Res>
    struct with_types_impl<0, 0, V, Res...> {
        template <typename T>
        static constexpr void execute(const T& fn) {
            fn.template operator()<Res...>();
        }
    };

    template <typename V, typename T>
    void with_types(const T& fn, size_t id1, size_t id2, size_t id3) {
        with_types_impl<3, 0, V>::execute(fn, id1, id2, id3);
    }
}


#define FLOAT float
#define DOUBLE double
#define FIXED(N, K) Fixed<N, K>
#define FAST_FIXED(N, K) FastFixed<N, K>

template <typename T>
struct TypeName {
    static std::string as_string() {
        return typeid(T).name();
    }
};

template <>
struct TypeName<float> {
    static std::string as_string() {
        return "FLOAT";
    }
};

template <>
struct TypeName<double> {
    static std::string as_string() {
        return "DOUBLE";
    }
};

template <size_t N, size_t K>
struct TypeName<Fixed<N, K>> {
    static std::string as_string() {
        return std::format("FIXED({},{})", N, K);
    }
};

template <size_t N, size_t K>
struct TypeName<FastFixed<N, K>> {
    static std::string as_string() {
        return std::format("FAST_FIXED({},{})", N, K);
    }
};


template <typename T, typename... Ts>
void fill_type_ids_for(std::map<std::string, size_t>& type_id) {
    static size_t id = 0;
    type_id[TypeName<T>::as_string()] = id++;

    if constexpr (sizeof...(Ts) > 0) {
        fill_type_ids_for<Ts...>(type_id);
    }
}


int main(int argc, char **argv) {
    cl::ParseCommandLineOptions(argc, argv);

    constexpr size_t N = 36, M = 84;
    char field[N][M + 1];
    std::ifstream input(InputFilenameArg);
    for (int i = 0; input.getline(field[i], M + 2) && i < N; ++i) {}

    std::map<std::string, size_t> type_id;
    fill_type_ids_for<TYPES>(type_id);
    const size_t pTypeId     = type_id[pTypeArg],
                 vTypeId     = type_id[vTypeArg],
                 vFlowTypeId = type_id[vFlowTypeArg];

    using types = meta::vector<TYPES>;
    meta::with_types<types>(fluid::run_simulation<N, M>(field),
        pTypeId, vTypeId, vFlowTypeId);
}