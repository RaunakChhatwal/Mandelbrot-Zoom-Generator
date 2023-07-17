#include <complex>
#include <gmpxx.h>
#include "params.hpp"
#include <png.h>

// The reason I'm wrapping the function in a class is because c++ doesn't allow template functions to be partially specialized.
template<int I>
class UnrolledLoop {
public:
    static std::complex<mpf_class> Body(const std::complex<mpf_class>& z, const std::complex<mpf_class>& c) {
        std::complex<mpf_class> prevIteration = UnrolledLoop<I-1>::Body(z, c);
        return prevIteration*prevIteration + c;
    }
};

template<>
class UnrolledLoop<0> {
public:
    // base case
    static std::complex<mpf_class> Body(const std::complex<mpf_class>& z, const std::complex<mpf_class>& c) { return z; }
};

char mandelbrot(std::complex<mpf_class> c) {
    std::complex<mpf_class> z({0, c.real().get_prec()}, {0, c.imag().get_prec()});

    int i;
    for (i = 0; z.real()*z.real() + z.imag()*z.imag() < 4.0 && i < max_iterations; i+=unroll_factor) {
        z = UnrolledLoop<unroll_factor>::Body(z, c);
    }

    return static_cast<char>((i * 255) / max_iterations);
}