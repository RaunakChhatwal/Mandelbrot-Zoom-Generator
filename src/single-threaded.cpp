#include <png.h>
#include <complex>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <vector>
#include <gmpxx.h>
#include <array>

constexpr int max_iterations = 1000;

int mandelbrot(std::complex<mpf_class> c) {
    std::complex<mpf_class> z(0, 0);

    int i;
    for (i = 0; z.real()*z.real() + z.imag()*z.imag() < 4.0 && i < max_iterations; i++) {
        z = z * z + c;
    }

    return (i * 255) / max_iterations;
}

std::complex<mpf_class> parseCompl(std::string, int widthPx, int heightPx);

int main(int argc, char** argv) {
    assert(argc == 5);

    int widthPx = std::atoi(std::string(argv[3], std::strchr(argv[3], 'x')).data());
    int heightPx = std::atoi(std::strchr(argv[3], 'x') + 1);

    std::complex bottomLeft = parseCompl(argv[1], widthPx, heightPx);
    std::complex topRight = parseCompl(argv[2], widthPx, heightPx);
    assert((bottomLeft.real() < topRight.real()) and (bottomLeft.imag() < topRight.imag()));

    assert(!std::strcmp(&argv[4][std::strlen(argv[4]) - 4], ".png"));

    FILE *fp = fopen(argv[4], "wb");
    if (!fp) {
        std::cerr << "Error: Couldn't open file for writing" << std::endl;
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, widthPx, heightPx,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    for (int y = 0; y < heightPx; y++) {
        std::vector<png_byte> row(3 * widthPx);

        for (int x = 0; x < widthPx; x++) {
            std::complex<mpf_class> c;
            c.real(bottomLeft.real() + (topRight.real() - bottomLeft.real()) * x / (double) widthPx);
            c.imag(topRight.imag() - (topRight.imag() - bottomLeft.imag()) * y / (double) heightPx);

            int shade = mandelbrot(c);
            row[3 * x] = row[3 * x + 1] = row[3 * x + 2] = shade;
        }
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, nullptr);
    fclose(fp);

    return 0;
}

std::complex<mpf_class> parseCompl(std::string arg, int widthPx, int heightPx) {
    std::complex<mpf_class> z;
    int i;
    while (arg[i] != '+')
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
