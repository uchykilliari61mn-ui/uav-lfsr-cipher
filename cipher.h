#ifndef CIPHER_H
#define CIPHER_H

#include <vector>
#include <cstdint>
#include "lfsr.h"

// --------------------------------------------------------------------------
// Шифрування кадру методом зі статті:
// «розбиття зображення на блоки + перемішування блоків за LFSR».
//
// Зображення ділиться на сітку rows x cols рівних блоків. За seed будується
// таблиця перестановок, і блоки міняються місцями згідно з нею.
//
// Напрямок перестановки:
//   encrypt: вихідний_блок[ perm[i] ] = вхідний_блок[ i ]
//   decrypt: вихідний_блок[ i ]       = вхідний_блок[ perm[i] ]
// (decrypt — точне обернення encrypt з тим самим seed)
// --------------------------------------------------------------------------

struct Image
{
    int width = 0;
    int height = 0;
    int channels = 0;              // 3 = RGB, 4 = RGBA
    std::vector<uint8_t> data;     // row-major, width*height*channels

    uint8_t* pixel(int x, int y)
    {
        return &data[((size_t)y * width + x) * channels];
    }
    const uint8_t* pixel(int x, int y) const
    {
        return &data[((size_t)y * width + x) * channels];
    }
};

// Копіює один блок (bw x bh) з позиції сітки (srcCol,srcRow) у (dstCol,dstRow).
inline void copyBlock(const Image& src, Image& dst,
                      int srcCol, int srcRow,
                      int dstCol, int dstRow,
                      int bw, int bh)
{
    for (int y = 0; y < bh; ++y)
    {
        const uint8_t* s = src.pixel(srcCol * bw, srcRow * bh + y);
        uint8_t* d = dst.pixel(dstCol * bw, dstRow * bh + y);
        std::copy(s, s + (size_t)bw * src.channels, d);
    }
}

// Спільне ядро: переставляє блоки за таблицею perm.
// Якщо invert == false — пряме шифрування, true — дешифрування.
inline Image scramble(const Image& src, int rows, int cols,
                      uint16_t seed, bool invert)
{
    // Розмір блоку; зайві пікселі з правого/нижнього краю відкидаються (crop),
    // як і зазначено у статті (videograbber теж обрізає кадр).
    int bw = src.width / cols;
    int bh = src.height / rows;
    int cropW = bw * cols;
    int cropH = bh * rows;

    Image out;
    out.width = cropW;
    out.height = cropH;
    out.channels = src.channels;
    out.data.assign((size_t)cropW * cropH * src.channels, 0);

    // Обрізана копія входу (щоб індексація блоків збігалася з виходом).
    Image in;
    in.width = cropW;
    in.height = cropH;
    in.channels = src.channels;
    in.data.assign((size_t)cropW * cropH * src.channels, 0);
    for (int y = 0; y < cropH; ++y)
    {
        const uint8_t* s = src.pixel(0, y);
        uint8_t* d = in.pixel(0, y);
        std::copy(s, s + (size_t)cropW * src.channels, d);
    }

    int n = rows * cols;
    std::vector<int> perm = buildPermutation(n, seed);

    for (int i = 0; i < n; ++i)
    {
        int srcIdx, dstIdx;
        if (!invert)
        {
            srcIdx = i;          // блок i ...
            dstIdx = perm[i];    // ... їде на позицію perm[i]
        }
        else
        {
            srcIdx = perm[i];    // обернено: з perm[i] ...
            dstIdx = i;          // ... повертаємо на i
        }
        copyBlock(in, out,
                  srcIdx % cols, srcIdx / cols,
                  dstIdx % cols, dstIdx / cols,
                  bw, bh);
    }
    return out;
}

inline Image encrypt(const Image& src, int rows, int cols, uint16_t seed)
{
    return scramble(src, rows, cols, seed, /*invert=*/false);
}

inline Image decrypt(const Image& src, int rows, int cols, uint16_t seed)
{
    return scramble(src, rows, cols, seed, /*invert=*/true);
}

#endif // CIPHER_H
