#include <png.h>
#include <string>
#include <tuple>

class PNG {
private:
    FILE* fp;
    png_structp png_ptr;
public:
    PNG(const std::string& png_name, int widthPx, int heightPx)
        :fp{fopen(png_name.c_str(), "wb")}, png_ptr{png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)}
    {
        if (!fp) {
            std::cerr << "Error: Couldn't open file for writing" << std::endl;
            exit(1);
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, widthPx, heightPx,
                    8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
    }

    void write_row(std::vector<std::tuple<char, char, char>> pixel_row) {   // literally a row of rgb pixels
        std::vector<png_byte> row(3 * pixel_row.size());
        for (int i = 0; i < pixel_row.size(); i++) {
            row[3*i] = std::get<0>(pixel_row[i]);
            row[3*i + 1] = std::get<1>(pixel_row[i]);
            row[3*i + 2] = std::get<2>(pixel_row[i]);
        }
        png_write_row(png_ptr, row.data());
    }

    ~PNG() {
        png_write_end(png_ptr, nullptr);
        fclose(fp);
    }
};