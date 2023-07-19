#include <complex>
#include <iostream>
#include <cassert>
#include <vector>
#include <gmpxx.h>
#include <thread>
#include <array>
#include <tuple>
#include "mandelbrot.hpp"
#include "GrayscaleBitmapImage.hpp"

std::tuple<std::complex<mpf_class>, std::complex<mpf_class>, int, int> parse(int argc, char** argv);

int main(int argc, char** argv) {
    // getting and parsing command line arguments
    std::complex<mpf_class> bottomLeft, topRight;
    int widthPx, heightPx;
    std::tie(bottomLeft, topRight, widthPx, heightPx) = parse(argc, argv);

    // getting the output png path and validating that it ends with ".png"
    std::string output_path{argv[4]};
    assert(output_path.size() >= 4 && std::string(output_path.end() - 4, output_path.end()) == ".png");

    // implementing a thread pool where there is no limit to the speed of incoming tasks but the number of running threads has a limit
    std::vector<std::thread> threads;
    GrayscaleBitmapImage img{widthPx, heightPx};
    auto unallocated{img.begin()};
    for (; unallocated + pixelsPerThread < img.end(); unallocated += pixelsPerThread) {
        thread_limit_sem.acquire();     // look to ./include/mandelbrot.hpp for the definition and purpose of the semaphore
        threads.push_back(std::thread(mandelbrot, unallocated, unallocated + pixelsPerThread, topRight, bottomLeft));
    }
    threads.push_back(std::thread(mandelbrot, unallocated, img.end(), topRight, bottomLeft));

    for (auto& thread : threads) {
        thread.join();
    }

    img.writeToPNG(output_path);

    return 0;
}

// this is to parse and decide on the precision of the input complex number
std::complex<mpf_class> parseCompl(std::string arg, int widthPx, int heightPx) {
    std::complex<mpf_class> z;
    int i = 0;
    while (arg.at(i) != '+')
        i++;
    
    std::string a{arg.begin(), arg.begin() + i};
    int precision1 = a.size() * std::log2(10);
    precision1 += std::log2(widthPx) + 52;

    assert(arg.back() == 'i');
    std::string b{arg.begin() + i + 1, arg.end() - 1};
    int precision2 = b.size() * std::log2(10);
    precision2 += std::log2(heightPx) + 52;

    z.real(mpf_class(a, precision1));
    z.imag(mpf_class(b, precision2));

    return z;
}

std::tuple<std::complex<mpf_class>, std::complex<mpf_class>, int, int> parse(int argc, char** argv) {
    assert(argc == 5);

    int widthPx = std::atoi(std::string(argv[3], std::strchr(argv[3], 'x')).data());
    int heightPx = std::atoi(std::strchr(argv[3], 'x') + 1);

    std::complex bottomLeft = parseCompl(argv[1], widthPx, heightPx);
    std::complex topRight = parseCompl(argv[2], widthPx, heightPx);
    assert((bottomLeft.real() < topRight.real()) and (bottomLeft.imag() < topRight.imag()));

    return {bottomLeft, topRight, widthPx, heightPx};
}
