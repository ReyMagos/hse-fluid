export module lazy_arithmetic;

namespace fluid {
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
    Add<A, B> operator+(const A &a, const B &b) {
        return Add<A, B>(a, b);
    }

    export template <typename A, typename B>
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
    Subtract<A, B> operator-(const A &a, const B &b) {
        return Subtract<A, B>(a, b);
    }

    export template <typename A, typename B>
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
    Multiply<A, B> operator*(const A &a, const B &b) {
        return Multiply<A, B>(a, b);
    }

    export template <typename A, typename B>
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
    Divide<A, B> operator/(const A &a, const B &b) {
        return Divide<A, B>(a, b);
    }

    export template <typename A, typename B>
    void operator/=(A &a, const B &b) {
        a = a / b;
    }
}
