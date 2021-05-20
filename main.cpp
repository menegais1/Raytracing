#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Libs/stb_image.h"
#include "Libs/stb_image_write.h"

#define byte unsigned char
std::string OUTPUT_PATH = "../Output/";


int main() {
    int width = 100;
    int height = 100;
    int channels = 3;
    int bytesLength = width * height * channels;
    byte* data = new byte[width * height * channels];

    for (int i = 0; i < bytesLength; i+=3) {
        data[i + 0] = 255;
        data[i + 1] = 255;
        data[i + 2] = 0;
    }
    stbi_write_png((OUTPUT_PATH + "teste.png").c_str(), 100, 100, 3, data, channels * sizeof(byte));

    return 0;
}
