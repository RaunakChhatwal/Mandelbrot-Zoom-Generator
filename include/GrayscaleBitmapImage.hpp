#ifndef GRAYSCALEBITMAPIMAGE_HPP
#define GRAYSCALEBITMAPIMAGE_HPP

#include <png.h>
#include <string>
#include <tuple>

// the bitmap image format is essentially a matrix of pixels
class GrayscaleBitmapImage {
private:
    char** pixels;
    int widthPx, heightPx;
public:
    GrayscaleBitmapImage(int _widthPx, int _heightPx)
        :pixels{new char*[_heightPx]}, widthPx{_widthPx}, heightPx{_heightPx}
    {
        pixels = new char*[_heightPx];
        for (int i = 0; i < _heightPx; i++) {
            pixels[i] = new char[_widthPx];
        }
    }
    
    struct Iterator {
        GrayscaleBitmapImage& img;
        int currRow, currCol;
        
        Iterator(GrayscaleBitmapImage& img, int currRow, int currCol)
            :img{img}, currRow{currRow}, currCol{currCol}
        { }

        char& operator*() const { return img.pixels[currRow][currCol]; }

        Iterator& operator++() {
            currRow += (currCol + 1 == img.getWidthPx());
            currCol = (currCol + 1)%img.getWidthPx();
            return *this;
        }

        // Postfix increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator operator+(long i) {
            Iterator res{img, currRow, currCol};

            if (currRow + i >= 0) {
                int currRowOffset = (currCol + i)/img.getWidthPx();
                res.currRow += currRowOffset;
                res.currCol += i - img.getWidthPx()*currRowOffset;
            } else {
                assert(false);
                int currRowOffset = (currRow + i)/img.getWidthPx() - 1;
                res.currRow += currRowOffset;
                res.currCol += i - img.getWidthPx()*currRowOffset;
            }
            return res;
        }

        Iterator& operator+=(long i) {
            auto res = *this + i;
            currRow = res.currRow;
            currCol = res.currCol;

            return *this;
        }

        friend bool operator<(const Iterator& a, const Iterator& b) {
            if (a.currRow == b.currRow) {
                return a.currCol < b.currCol;
            } else {
                return a.currRow < b.currRow;
            }
        }

        friend bool operator>(const Iterator& a, const Iterator& b) {
            return a != b && !(a<b);
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.currRow == b.currRow && a.currCol == b.currCol;
        }

        friend bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); }
    };

    Iterator begin() {
        return Iterator(*this, 0, 0);
    }
    Iterator end() {
        return Iterator(*this, heightPx - 1, widthPx - 1);
    }

    int getWidthPx() const {
        return widthPx;
    }

    int getHeightPx() const {
        return heightPx;
    }

    void writeToPNG(const std::string& imgName) const {
        FILE* fp = fopen(imgName.c_str(), "wb");
        if (!fp) {
            std::cerr << "Error: Couldn't open file for writing" << std::endl;
            exit(1);
        }

        png_structp png_ptr{png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)};
        png_infop info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, widthPx, heightPx,
                    8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        for (int y = 0; y < heightPx; y++) {
            std::vector<png_byte> rgbPixelRow(3*widthPx);
            for (int x = 0; x < widthPx; x++) {
                rgbPixelRow[3*x] = rgbPixelRow[3*x + 1] = rgbPixelRow[3*x + 2] = pixels[y][x];
            }

            png_write_row(png_ptr, rgbPixelRow.data());
        }

        png_write_end(png_ptr, nullptr);
        fclose(fp);
    }

    ~GrayscaleBitmapImage() {
        for (int i = 0; i < heightPx; i++) {
            delete[] pixels[i];
        }

        delete[] pixels;
    }
};

#endif // GRAYSCALEBITMAPIMAGE_HPP