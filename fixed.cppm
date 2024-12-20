module;

#include <cstdint>
#include <ostream>

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

    explicit operator float() const {
        return v / static_cast<float>(1 << K);
    }

    explicit operator double() const {
        return v / static_cast<double>(1 << K);
    }
};

template <typename intN_t, unsigned K>
std::ostream &operator<<(std::ostream &out, const Fixed<intN_t, K> &f) {
    return out << f.v / static_cast<double>(1 << K);
}
