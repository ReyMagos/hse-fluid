module;

#include <format>
#include <stdexcept>

export module meta;

namespace meta {
    /**
     * Vector type.
     * Dummy object that holds sequence of types
     *
     * @tparam Ts Types
     *
     * @namespace meta
     */
    export template <typename... Ts>
    struct vector {};


    template <typename V, size_t N>
    struct at_impl;

    template <template <typename...> typename V, typename T, typename... Ts, size_t N>
    struct at_impl<V<T, Ts...>, N> {
        static_assert(N <= sizeof...(Ts), "Index is out of range");
        using type = typename at_impl<V<Ts...>, N - 1>::type;
    };

    template <template <typename...> typename V, typename T, typename... Ts>
    struct at_impl<V<T, Ts...>, 0> {
        using type = T;
    };

    /**
     * Returns type from vector V at index N
     *
     * @tparam V Vector type
     * @tparam N Index
     *
     * @namespace meta
     */
    export template <typename V, size_t N>
    using at = typename at_impl<V, N>::type;


    template <size_t D, size_t N, typename V, typename... Res>
    struct with_types_impl {
        template <typename T, typename... IDs>
        static constexpr void execute(const T& fn, size_t id, IDs... ids) {
            if (N == id) {
                with_types_impl<D - 1, 0, V, Res..., at<V, N>>::execute(fn, ids...);
            } else {
                with_types_impl<D, N + 1, V, Res...>::execute(fn, id, ids...);
            }
        }
    };

    template <size_t D, template <typename...> typename V, typename... Ts, typename... Res>
    struct with_types_impl<D, sizeof...(Ts), V<Ts...>, Res...> {
        template <typename T, typename... IDs>
        static constexpr void execute(const T& fn, size_t id, IDs... ids) {
            throw std::invalid_argument(
                std::format("Type with ID {} doesn't exists", id)
            );
        }
    };

    template <typename V, typename... Res>
    struct with_types_impl<0, 0, V, Res...> {
        template <typename T>
        static constexpr void execute(const T& fn) {
            fn.template operator()<Res...>();
        }
    };

    /**
     * Calls templated function `fn<T1, T2, ..., TN>` with types from vector `V`
     * specified by `id1, id2, ..., idN` respectively.
     * Roughly equivalent to `fn<at<V, id1>, at<V, id2>, ..., at<V, idN>>()`
     *
     * Version for N = 3 dimensions
     *
     * @tparam V Vector type
     * @tparam F Function type
     *
     * @param fn Function
     * @param id1,id2,id3 IDs of selected types
     *
     * @see meta::at<V, N>
     * @namespace meta
     */
    export template <typename V, typename F>
    void with_types(const F& fn, size_t id1, size_t id2, size_t id3) {
        with_types_impl<3, 0, V>::execute(fn, id1, id2, id3);
    }
}
