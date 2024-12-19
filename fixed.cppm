module;

#include <cstdint>
#include <ostream>

#define TYPES float, double, Fixed<int32_t, 16>

export module fixed;

export template <typename intN_t, unsigned K>
struct Fixed {
    intN_t v;

    constexpr Fixed(const int v): v(v << K) {}
    constexpr Fixed(const float f): v(f * (1 << K)) {}
    constexpr Fixed(const double f): v(f * (1 << K)) {}
    constexpr Fixed(): v(0) {}

    static constexpr Fixed from_raw(intN_t x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const Fixed&) const = default;
    bool operator==(const Fixed&) const = default;

    Fixed operator+(const Fixed &other) const {
        // TODO: Why don't just use constructor?
        return from_raw(v + other.v);
    }

    Fixed operator-(const Fixed other) const {
        return from_raw(v - other.v);
    }

    Fixed operator*(const Fixed other) const {
        return from_raw((static_cast<int64_t>(v) * other.v) >> K);
    }

    Fixed operator/(const Fixed other) const {
        return from_raw((static_cast<int64_t>(v) << K) / other.v);
    }

    Fixed &operator+=(const Fixed other) {
        return *this = *this + other;
    }

    Fixed &operator-=(const Fixed other) {
        return *this = *this - other;
    }

    Fixed &operator*=(const Fixed other) {
        return *this = *this * other;
    }

    Fixed &operator/=(const Fixed other) {
        return *this = *this / other;
    }

    Fixed operator-() const {
        return from_raw(-v);
    }

    static Fixed abs(Fixed x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    operator float() const {
        return v / static_cast<float>(1 << K);
    }

    operator double() const {
        return v / static_cast<double>(1 << K);
    }
};


template <typename T, typename... Types>
concept is_one_of = std::disjunction_v<std::is_same<T, Types>...>;

template <typename A, typename B>
struct Add {
    A a;
    B b;
    constexpr Add(A a, B b): a(a), b(b) {}

    template <typename T>
    operator T() const {
        return static_cast<T>(a) + static_cast<T>(b);
    }
};

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
Add<A, B> operator+(const A &a, const B &b) {
    return Add<A, B>(a, b);
}

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
void operator+=(A &a, const B &b) {
    a = a + b;
}

template <typename A, typename B>
struct Subtract {
    A a;
    B b;
    constexpr Subtract(A a, B b): a(a), b(b) {}

    template <typename T>
    operator T() const {
        return static_cast<T>(a) - static_cast<T>(b);
    }
};

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
Subtract<A, B> operator-(const A &a, const B &b) {
    return Subtract<A, B>(a, b);
}

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
void operator-=(A &a, const B &b) {
    a = a - b;
}

template <typename A, typename B>
struct Multiply {
    A a;
    B b;
    constexpr Multiply(A a, B b): a(a), b(b) {}

    template <typename T>
    operator T() const {
        return static_cast<T>(a) * static_cast<T>(b);
    }
};

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
Multiply<A, B> operator*(const A &a, const B &b) {
    return Multiply<A, B>(a, b);
}

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
void operator*=(A &a, const B &b) {
    a = a * b;
}

template <typename A, typename B>
struct Divide {
    A a;
    B b;
    constexpr Divide(A a, B b): a(a), b(b) {}

    template <typename T>
    operator T() const {
        return static_cast<T>(a) / static_cast<T>(b);
    }
};

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
Divide<A, B> operator/(const A &a, const B &b) {
    return Divide<A, B>(a, b);
}

export template <typename A, typename B>
requires is_one_of<A, TYPES> && is_one_of<B, TYPES>
void operator/=(A &a, const B &b) {
    a = a / b;
}

template <typename intN_t, unsigned K>
std::ostream &operator<<(std::ostream &out, const Fixed<intN_t, K> &f) {
    return out << f.v / static_cast<double>(1 << K);
}
