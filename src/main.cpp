#include "pbr2gltf2.hpp"
#include <cstdio>
#include <fstream>
#include <string>

float clampf(float x, float minVal, float maxVal)
{
    float t = (x > minVal) ? x : minVal;

    return (t < maxVal) ? t : maxVal;
}

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        printf("Usage: pbr2gltf2 folder [-m 1.0 -r 1.0 -n true -e true]\n");

        return 0;
    }

    //

    float defaultMetallicFactor = 1.0f;
    float defaultRoughnessFactor = 1.0f;
    bool keepNormalImageData = true;
    bool keepEmissiveImageData = true;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && (i + 1 < argc)) {
            defaultMetallicFactor = clampf(std::stof(argv[i + 1]), 0.0f, 1.0f);
        } else if (strcmp(argv[i], "-r") == 0 && (i + 1 < argc)) {
            defaultRoughnessFactor = clampf(std::stof(argv[i + 1]), 0.0f, 1.0f);
        } else if (strcmp(argv[i], "-n") == 0 && (i + 1 < argc)) {
            if (strcmp(argv[i + 1], "true") == 0) {
                keepNormalImageData = true;
            } else if (strcmp(argv[i + 1], "false") == 0) {
                keepNormalImageData = false;
            }
        } else if (strcmp(argv[i], "-e") == 0 && (i + 1 < argc)) {
            if (strcmp(argv[i + 1], "true") == 0) {
                keepEmissiveImageData = true;
            } else if (strcmp(argv[i + 1], "false") == 0) {
                keepEmissiveImageData = false;
            }
        }
    }

    std::string path = argv[1];

    pbr2gltf2::convert_to_filesystem(path, "", defaultMetallicFactor, defaultRoughnessFactor, keepNormalImageData, keepEmissiveImageData);
}
