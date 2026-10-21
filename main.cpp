// ---------------------------------------------------------------------------
// Шифратор кадру для аналогової відеопередачі з дрона (UAV),
// за статтею: "Low Cost Analog Video Transmission Security of UAV
// based on Linear Feedback Shift Register (LFSR)", ISITIA 2021.
//
// Метод: кадр ділиться на сітку блоків, блоки перемішуються за
// псевдовипадковою перестановкою, згенерованою 16-бітним LFSR із seed-ключа.
//
// Використання:
//   uav_cipher encrypt <in.png>  <out.png> <seed> [rows] [cols]
//   uav_cipher decrypt <in.png>  <out.png> <seed> [rows] [cols]
//
// rows, cols за замовчуванням 3 і 4 (12 блоків, як у статті).
// ---------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

#include "cipher.h"

static bool loadImage(const char* path, Image& img)
{
    int w, h, c;
    uint8_t* raw = stbi_load(path, &w, &h, &c, 0);
    if (!raw)
    {
        std::fprintf(stderr, "Помилка: не вдалося відкрити '%s'\n", path);
        return false;
    }
    img.width = w;
    img.height = h;
    img.channels = c;
    img.data.assign(raw, raw + (size_t)w * h * c);
    stbi_image_free(raw);
    return true;
}

static bool savePng(const char* path, const Image& img)
{
    int ok = stbi_write_png(path, img.width, img.height, img.channels,
                            img.data.data(), img.width * img.channels);
    if (!ok)
        std::fprintf(stderr, "Помилка: не вдалося записати '%s'\n", path);
    return ok != 0;
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        std::fprintf(stderr,
            "Використання:\n"
            "  %s encrypt <in.png> <out.png> <seed> [rows] [cols]\n"
            "  %s decrypt <in.png> <out.png> <seed> [rows] [cols]\n",
            argv[0], argv[0]);
        return 1;
    }

    std::string mode = argv[1];
    const char* inPath = argv[2];
    const char* outPath = argv[3];
    uint16_t seed = (uint16_t)std::strtoul(argv[4], nullptr, 10);
    int rows = (argc > 5) ? std::atoi(argv[5]) : 3;
    int cols = (argc > 6) ? std::atoi(argv[6]) : 4;

    if (rows < 1 || cols < 1)
    {
        std::fprintf(stderr, "Помилка: rows і cols мають бути >= 1\n");
        return 1;
    }

    Image in;
    if (!loadImage(inPath, in))
        return 1;

    Image out;
    if (mode == "encrypt")
        out = encrypt(in, rows, cols, seed);
    else if (mode == "decrypt")
        out = decrypt(in, rows, cols, seed);
    else
    {
        std::fprintf(stderr, "Невідомий режим '%s' (encrypt|decrypt)\n", mode.c_str());
        return 1;
    }

    if (!savePng(outPath, out))
        return 1;

    std::printf("%s: %s -> %s  (seed=%u, grid=%dx%d, %d блоків)\n",
                mode.c_str(), inPath, outPath, seed, rows, cols, rows * cols);
    return 0;
}
