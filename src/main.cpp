#include <complex>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <cstring>
#include <vector>
#include <gmpxx.h>
#include <thread>
#include <array>
#include <tuple>
#include "mandelbrot.hpp"
#include "PNG.hpp"

void pngInit();
std::tuple<std::complex<mpf_class>, std::complex<mpf_class>, int, int> parse(int argc, char** argv);
FILE* pngInit(const std::string& filename);

int main(int argc, char** argv) {
    std::complex<mpf_class> bottomLeft, topRight;
    int widthPx, heightPx;
    std::tie(bottomLeft, topRight, widthPx, heightPx) = parse(argc, argv);

    std::string output_name{argv[4]};
    assert(output_name.size() >= 4 && std::string(output_name.end() - 4, output_name.end()) == ".png");
    auto output_png = PNG(output_name, widthPx, heightPx);

    for (int y = 0; y < heightPx; y++) {
        std::vector<std::tuple<char, char, char>> pixel_row(widthPx);

        std::array<std::thread, 100> threads;
        for (int i = 0; i < threads.size(); i++) {
            threads[i] = std::thread([&, i]() {
                for (int x = widthPx * i / threads.size(); x < widthPx * (i + 1) / threads.size(); x++) {
                    std::complex<mpf_class> c;
                    c.real(bottomLeft.real() + (topRight.real() - bottomLeft.real()) * x / (double) widthPx);
                    c.imag(topRight.imag() - (topRight.imag() - bottomLeft.imag()) * y / (double) heightPx);

                    int shade = mandelbrot(c);
                    pixel_row[x] = {shade, shade, shade};
                }
            });
        }
        for (std::thread& t : threads) {
            t.join();
        }
        output_png.write_row(pixel_row);
    }

    return 0;
}

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
