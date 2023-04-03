#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

namespace ABraveFish {
#pragma pack(push, 1)
struct TGA_Header {
    char  idlength;
    char  colormaptype;
    char  datatypecode;
    short colormaporigin;
    short colormaplength;
    char  colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
    union {
        struct {
            unsigned char r, g, b, a;
        };
        unsigned char bgra[4];
        unsigned int  val;
    };
    int32_t bytespp;

    TGAColor()
        : val(0)
        , bytespp(1) {}

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A = 255.f)
        : r(R)
        , g(G)
        , b(B)
        , a(A)
        , bytespp(4) {}

    TGAColor(int32_t v, int32_t bpp)
        : val(v)
        , bytespp(bpp) {}

    TGAColor(const TGAColor& c)
        : val(c.val)
        , bytespp(c.bytespp) {}

    TGAColor(const unsigned char* p, int32_t bpp)
        : val(0)
        , bytespp(bpp) {
        for (int32_t i = 0; i < bpp; i++) {
            bgra[i] = p[i];
        }
    }

    TGAColor& operator=(const TGAColor& c) {
        if (this != &c) {
            bytespp = c.bytespp;
            val     = c.val;
        }
        return *this;
    }
};

class TGAImage {
protected:
    unsigned char* data;
    int32_t        width;
    int32_t        height;
    int32_t        bytespp;

    bool load_rle_data(std::ifstream& in);
    bool unload_rle_data(std::ofstream& out);

public:
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

    TGAImage();
    TGAImage(int32_t w, int32_t h, int32_t bpp);
    TGAImage(const TGAImage& img);
    bool     read_tga_file(const char* filename);
    bool     write_tga_file(const char* filename, bool rle = true);
    bool     flip_horizontally();
    bool     flip_vertically();
    bool     scale(int32_t w, int32_t h);
    TGAColor get(int32_t x, int32_t y);
    bool     set(int32_t x, int32_t y, TGAColor c);
    ~TGAImage();
    TGAImage&      operator=(const TGAImage& img);
    float          get_width();
    float          get_height();
    int32_t        get_bytespp();
    unsigned char* buffer();
    void           clear();
};

} // namespace ABraveFish
#endif //__IMAGE_H__
