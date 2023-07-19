#ifndef MANDELBROT_HPP
#define MANDELBROT_HPP

#include <complex>
#include <vector>
#include <gmpxx.h>
#include <array>
#include <semaphore>
#include <iostream>
#include "params.hpp"
#include "GrayscaleBitmapImage.hpp"

/*
    This counting semaphore enforces the limit defined in ./include/params.hpp on the number of active threads. Its counter is initialized to that limit (contained
    in the variable numThreads) and the counter is decremented each time main.cpp starts a new thread (see main.cpp line 29). After launching 
    numThreads threads, the counter is empty and the semaphore blocks any more threads from being created. When a thread finishes its task, it increments the
    counter in the semaphore (see line 59 of this file), allowing a new thread to be launched in its place.
*/
std::counting_semaphore<numThreads> thread_limit_sem{numThreads};

// The function is wrapped in a class because c++ doesn't allow template functions to be partially specialized, only classes.
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
    static std::complex<mpf_class> Body(const std::complex<mpf_class>& z, const std::complex<mpf_class>&) { return z; }
};

void mandelbrot(
    GrayscaleBitmapImage::Iterator begin,
    GrayscaleBitmapImage::Iterator end,
    std::complex<mpf_class> topRight,
    std::complex<mpf_class> bottomLeft
) {
    // sequentially calculates the shade of each pixel referred to by iterators begin to end
    for (auto currPixel = begin; currPixel < end; ++currPixel) {
        // c refers to the complex number which corresponds to currPixel
        std::complex<mpf_class> c;
        c.real(bottomLeft.real() + (topRight.real() - bottomLeft.real()) * currPixel.currCol / (double) currPixel.img.getWidthPx());
        c.imag(topRight.imag() - (topRight.imag() - bottomLeft.imag()) * currPixel.currRow / (double) currPixel.img.getHeightPx());

        std::complex<mpf_class> z({0, c.real().get_prec()}, {0, c.imag().get_prec()});

        int i;
        for (i = 0; z.real()*z.real() + z.imag()*z.imag() < 4.0 && i < iterations; i+=unrollFactor) {
            z = UnrolledLoop<unrollFactor>::Body(z, c);
        }

        *currPixel = static_cast<char>((i * 255) / iterations);
    }

    thread_limit_sem.release();
}

#endif // MANDELBROT_HPP