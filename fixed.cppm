module;

#include <cstdint>
#include <format>
#include <ostream>

export module fixed;

template <typename intN_t, unsigned K, typename T>
struct FixedImpl {
    intN_t v;

    constexpr FixedImpl(const int v): v(v << K) {}
    constexpr FixedImpl(const float f): v(f * (1 << K)) {}
    constexpr FixedImpl(const double f): v(f * (1 << K)) {}
    constexpr FixedImpl(): v(0) {}

    auto operator<=>(const FixedImpl&) const = default;
    bool operator==(const FixedImpl&) const = default;

    T operator+(const T &other) const {
        return T(v + other.v);
    }

    T operator-(const T other) const {
        return T(v - other.v);
    }

    T operator*(const T other) const {
        intN_t res = (static_cast<int64_t>(v) * other.v) >> K;
        return T(res);
    }

    T operator/(const T other) const {
        intN_t res = (static_cast<int64_t>(v) << K) / other.v;
        return T(res);
    }

    T &operator+=(const T other) {
        return *this = *this + other;
    }

    T &operator-=(const T other) {
        return *this = *this - other;
    }

    T &operator*=(const T other) {
        return *this = *this * other;
    }

    T &operator/=(const T other) {
        return *this = *this / other;
    }

    T operator-() const {
        return T(-v);
    }

    static T abs(T x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    explicit operator float() const {
        return v / static_cast<float>(1 << K);
    }

    explicit operator double() const {
        return v / static_cast<double>(1 << K);
    }
};

template <typename intN_t, unsigned K, typename T>
std::ostream &operator<<(std::ostream &out, const FixedImpl<intN_t, K, T> &f) {
    return out << f.v / static_cast<double>(1 << K);
}

template <size_t N> requires (N <= INTMAX_WIDTH)
struct int_type;

template <>
struct int_type<8> {
    using type = int8_t;
};

template <>
struct int_type<16> {
    using type = int16_t;
};

template <>
struct int_type<32> {
    using type = int32_t;
};

template <>
struct int_type<64> {
    using type = int64_t;
};


template <size_t N> requires (N <= INTMAX_WIDTH)
struct fast_int_type;

template <>
struct fast_int_type<8> {
    using type = int_fast8_t;
};

template <>
struct fast_int_type<16> {
    using type = int_fast16_t;
};

template <>
struct fast_int_type<32> {
    using type = int_fast32_t;
};

template <>
struct fast_int_type<64> {
    using type = int_fast64_t;
};

export template <typename T>
struct TypeName {
    static constexpr std::string as_string() {
        return typeid(T).name();
    }
};

template <>
struct TypeName<float> {
    static constexpr std::string as_string() {
        return "FLOAT";
    }
};

template <>
struct TypeName<double> {
    static constexpr std::string as_string() {
        return "DOUBLE";
    }
};

// TODO: Awful constructor inheritance
export template <size_t N, size_t K>
struct Fixed: FixedImpl<typename int_type<N>::type, K, Fixed<N, K>>, TypeName<Fixed<N, K>> {
    constexpr Fixed(const int v): FixedImpl<typename int_type<N>::type, K, Fixed>(v) {};
    constexpr Fixed(const float f): FixedImpl<typename int_type<N>::type, K, Fixed>(f) {}
    constexpr Fixed(const double f): FixedImpl<typename int_type<N>::type, K, Fixed>(f) {}
    constexpr Fixed(): FixedImpl<typename int_type<N>::type, K, Fixed>(0) {}

    static constexpr std::string as_string() {
        return std::format("FIXED({},{})", N, K);
    }
};

export template <size_t N, size_t K>
struct FastFixed: FixedImpl<typename fast_int_type<N>::type, K, FastFixed<N, K>>, TypeName<FastFixed<N, K>> {
    constexpr FastFixed(const int v): FixedImpl<typename fast_int_type<N>::type, K, FastFixed>(v) {};
    constexpr FastFixed(const float f): FixedImpl<typename fast_int_type<N>::type, K, FastFixed>(f) {}
    constexpr FastFixed(const double f): FixedImpl<typename fast_int_type<N>::type, K, FastFixed>(f) {}
    constexpr FastFixed(): FixedImpl<typename fast_int_type<N>::type, K, FastFixed>(0) {}

    static constexpr std::string as_string() {
        return std::format("FAST_FIXED({},{})", N, K);
    }
};
