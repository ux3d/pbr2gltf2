
#include "pbr2gltf2.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>

#include <vector>

#include <nlohmann/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifdef __APPLE__
#include <Availability.h> // for deployment target to support pre-catalina targets without std::fs
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (defined(__cplusplus) && __cplusplus >= 201703L)) && defined(__has_include)
#if __has_include(<filesystem>) && (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif
#endif
#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif

using json = nlohmann::json;

struct DecomposedPath {
    std::string parentPath = "";
    std::string stem = "";
    std::string extension = "";
};

struct ImageDataResource {
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;
};

std::string toLowercase(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

DecomposedPath decomposePath(const std::string& path)
{
    fs::path filesystemPath(path);
    DecomposedPath decomposedPath;

    decomposedPath.parentPath = filesystemPath.parent_path().generic_string();
    decomposedPath.stem = filesystemPath.stem().generic_string();
    decomposedPath.extension = filesystemPath.extension().generic_string();

    return decomposedPath;
}

bool loadImage(ImageDataResource& imageDataResource, const std::string& filename)
{
    DecomposedPath decomposedPath = decomposePath(filename);

    int x = 0;
    int y = 0;
    int comp = 0;
    int req_comp = 0;

    uint8_t* tempData = static_cast<uint8_t*>(stbi_load(filename.c_str(), &x, &y, &comp, req_comp));
    if (!tempData) {
        return false;
    }

    imageDataResource.width = static_cast<uint32_t>(x);
    imageDataResource.height = static_cast<uint32_t>(y);
    imageDataResource.channels = static_cast<uint32_t>(comp);
    imageDataResource.pixels.resize(imageDataResource.width * imageDataResource.height * imageDataResource.channels);
    memcpy(imageDataResource.pixels.data(), tempData, imageDataResource.width * imageDataResource.height * imageDataResource.channels);

    free(tempData);

    return true;
}

bool loadFile(std::string& output, const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);

    output.resize(fileSize);

    file.read(output.data(), fileSize);
    file.close();

    return true;
}

bool saveFile(const std::string& output, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(output.data(), output.size());
    file.close();

    return true;
}

size_t gatherStem(const std::string& stem)
{
    size_t result = std::string::npos;

    result = toLowercase(stem).find("_color");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_base_color");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_basecolor");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_base color");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_opacity");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_metallic");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_roughness");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_rough");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_ao");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_ambientocclusion");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_normal");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_nor");
    if (result != std::string::npos) {
        return result;
    }

    result = toLowercase(stem).find("_emissive");
    if (result != std::string::npos) {
        return result;
    }

    return result;
}

int pbr2gltf2::convert_to_filesystem(const std::string& pbrPath, const std::string& gltfPath, float defaultMetallicFactor, float defaultRoughnessFactor, bool keepNormalImageData, bool keepEmissiveImageData)
{
    std::string stem = "pbr";

    json glTF = json::object();

    json asset = json::object();
    asset["version"] = "2.0";
    asset["generator"] = "pbr2gltf2 by UX3D";

    glTF["asset"] = asset;

    json images = json::array();
    json textures = json::array();
    json materials = json::array();

    json material = json::object();
    json pbrMetallicRoughness = json::object();

    //

    bool init = true;

    bool writeBaseColor = false;
    bool writeOpacity = false;
    bool writeMetallic = false;
    bool writeRoughness = false;
    bool writeOcclusion = false;
    bool writeNormal = false;
    bool writeEmissive = false;

    ImageDataResource baseColorImage;

    ImageDataResource metallicRoughnessImage;

    ImageDataResource normalImage;

    std::string normalImageRaw;
    std::string normalImageRawExtension;

    ImageDataResource emissiveImage;

    std::string emissiveImageRaw;
    std::string emissiveImageRawExtension;

    for (const auto& directoryEntry : fs::directory_iterator(pbrPath)) {
        std::string filename = directoryEntry.path().generic_string();

        printf("Info: Processing '%s'\n", filename.c_str());

        DecomposedPath decomposedPath = decomposePath(filename);

        std::string lowercaseExtension = toLowercase(decomposedPath.extension);
        if (!(lowercaseExtension == ".png" || lowercaseExtension == ".jpg" || lowercaseExtension == ".jpeg")) {
            continue;
        }

        ImageDataResource imageDataResource;
        if (!loadImage(imageDataResource, filename)) {
            printf("Warning: Skipping image '%s' because could not load size\n", filename.c_str());

            continue;
        }

        if (init) {
            auto it = gatherStem(decomposedPath.stem);
            if (it != std::string::npos) {
                stem = decomposedPath.stem.substr(0, it);
            }

            //

            baseColorImage.width = imageDataResource.width;
            baseColorImage.height = imageDataResource.height;
            baseColorImage.channels = 4;
            baseColorImage.pixels.resize(baseColorImage.channels * baseColorImage.width * baseColorImage.height);

            for (size_t y = 0; y < baseColorImage.height; y++) {
                for (size_t x = 0; x < baseColorImage.width; x++) {
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 0] = 255;
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 1] = 255;
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 2] = 255;
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 3] = 255;
                }
            }

            //

            metallicRoughnessImage.width = imageDataResource.width;
            metallicRoughnessImage.height = imageDataResource.height;
            metallicRoughnessImage.channels = 4;
            metallicRoughnessImage.pixels.resize(metallicRoughnessImage.channels * metallicRoughnessImage.width * metallicRoughnessImage.height);

            for (size_t y = 0; y < metallicRoughnessImage.height; y++) {
                for (size_t x = 0; x < metallicRoughnessImage.width; x++) {
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 0] = 255;
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 1] = 255;
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 2] = 255;
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 3] = 255;
                }
            }

            if (keepNormalImageData) {
                // Keeping original byte date
            } else {
                normalImage.width = imageDataResource.width;
                normalImage.height = imageDataResource.height;
                normalImage.channels = 3;
                normalImage.pixels.resize(normalImage.channels * normalImage.width * normalImage.height);

                // Not required for normal map
            }

            if (keepEmissiveImageData) {
                // Keeping original byte date
            } else {
                emissiveImage.width = imageDataResource.width;
                emissiveImage.height = imageDataResource.height;
                emissiveImage.channels = 3;
                emissiveImage.pixels.resize(emissiveImage.channels * emissiveImage.width * emissiveImage.height);

                // Not required for normal map
            }

            //

            init = false;
        } else {
            if ((imageDataResource.width != baseColorImage.width) || (imageDataResource.height != baseColorImage.height)) {
                printf("Warning: Skipping image '%s' because of size\n", filename.c_str());

                continue;
            }
        }

        //

        bool hasBaseColor = ((toLowercase(filename).find("_color.") != std::string::npos) || (toLowercase(filename).find("_base_color.") != std::string::npos) || (toLowercase(filename).find("_basecolor.") != std::string::npos) || (toLowercase(filename).find("_base color.") != std::string::npos));
        if (hasBaseColor) {
            for (size_t y = 0; y < baseColorImage.height; y++) {
                for (size_t x = 0; x < baseColorImage.width; x++) {
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 1];
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 2];
                }
            }

            printf("Info: Found base color\n");

            writeBaseColor = true;
        }

        bool hasOpacity = (toLowercase(filename).find("_opacity.") != std::string::npos);
        if (hasOpacity) {
            for (size_t y = 0; y < baseColorImage.height; y++) {
                for (size_t x = 0; x < baseColorImage.width; x++) {
                    baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 3] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                }
            }

            printf("Info: Found alpha\n");

            writeOpacity = true;
        }

        bool hasMetallic = (toLowercase(filename).find("_metallic.") != std::string::npos);
        if (hasMetallic) {
            for (size_t y = 0; y < metallicRoughnessImage.height; y++) {
                for (size_t x = 0; x < metallicRoughnessImage.width; x++) {
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                }
            }

            printf("Info: Found metallic\n");

            writeMetallic = true;
        }

        bool hasRoughness = ((toLowercase(filename).find("_roughness.") != std::string::npos) || (toLowercase(filename).find("_rough_") != std::string::npos));
        if (hasRoughness) {
            for (size_t y = 0; y < metallicRoughnessImage.height; y++) {
                for (size_t x = 0; x < metallicRoughnessImage.width; x++) {
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                }
            }

            printf("Info: Found roughness\n");

            writeRoughness = true;
        }

        bool hasOcclusion = ((toLowercase(filename).find("_ao.") != std::string::npos) || (toLowercase(filename).find("_ambientocclusion.") != std::string::npos) || (toLowercase(filename).find("_ao_") != std::string::npos));
        if (hasOcclusion) {
            for (size_t y = 0; y < metallicRoughnessImage.height; y++) {
                for (size_t x = 0; x < metallicRoughnessImage.width; x++) {
                    metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                }
            }

            printf("Info: Found occlusion\n");

            writeOcclusion = true;
        }

        bool hasNormal = ((toLowercase(filename).find("_normal.") != std::string::npos) || (toLowercase(filename).find("_nor_") != std::string::npos));
        if (hasNormal) {
            if (keepNormalImageData) {
                if (!loadFile(normalImageRaw, filename)) {
                    printf("Error: Could not load image raw '%s'\n", filename.c_str());

                    return -1;
                }

                normalImageRawExtension = decomposedPath.extension;
            } else {
                for (size_t y = 0; y < normalImage.height; y++) {
                    for (size_t x = 0; x < normalImage.width; x++) {
                        normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                        normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 1];
                        normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 2];
                    }
                }
            }

            printf("Info: Found normal\n");

            writeNormal = true;
        }

        bool hasEmissive = (toLowercase(filename).find("_emissive.") != std::string::npos);
        if (hasEmissive) {
            if (keepEmissiveImageData) {
                if (!loadFile(emissiveImageRaw, filename)) {
                    printf("Error: Could not load image raw '%s'\n", filename.c_str());

                    return -1;
                }

                emissiveImageRawExtension = decomposedPath.extension;
            } else {
                for (size_t y = 0; y < emissiveImage.height; y++) {
                    for (size_t x = 0; x < emissiveImage.width; x++) {
                        emissiveImage.pixels.data()[y * emissiveImage.width * emissiveImage.channels + x * emissiveImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
                        emissiveImage.pixels.data()[y * emissiveImage.width * emissiveImage.channels + x * emissiveImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 1];
                        emissiveImage.pixels.data()[y * emissiveImage.width * emissiveImage.channels + x * emissiveImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 2];
                    }
                }
            }

            printf("Info: Found emissive\n");

            writeEmissive = true;
        }
    }

    if (writeBaseColor || writeOpacity) {
        std::string imagePath = gltfPath + "/" + stem + "_baseColor.png";
        if (!stbi_write_png(imagePath.c_str(), baseColorImage.width, baseColorImage.height, baseColorImage.channels, baseColorImage.pixels.data(), 0)) {
            printf("Error: Could not save image '%s'\n", imagePath.c_str());

            return -1;
        }

        size_t index = textures.size();

        json baseColorTexture = json::object();
        baseColorTexture["index"] = index;

        pbrMetallicRoughness["baseColorTexture"] = baseColorTexture;

        if (!writeOpacity) {
            // Do nothing
        } else {
            material["alphaMode"] = "MASK";
            material["doubleSided"] = true;
        }

        json texture = json::object();
        texture["source"] = index;
        textures.push_back(texture);

        json image = json::object();
        image["uri"] = imagePath;
        images.push_back(image);
    }

    if (writeMetallic || writeRoughness || writeOcclusion) {
        std::string imagePath = gltfPath + "/" + stem + "_metallicRoughness.png";
        if (!stbi_write_png(imagePath.c_str(), metallicRoughnessImage.width, metallicRoughnessImage.height, metallicRoughnessImage.channels, metallicRoughnessImage.pixels.data(), 0)) {
            printf("Error: Could not save image '%s'\n", imagePath.c_str());

            return -1;
        }

        size_t index = textures.size();

        json metallicRoughnessTexture = json::object();
        metallicRoughnessTexture["index"] = index;

        if (!writeMetallic) {
            pbrMetallicRoughness["metallicFactor"] = defaultMetallicFactor;
        }
        if (!writeRoughness) {
            pbrMetallicRoughness["roughnessFactor"] = defaultRoughnessFactor;
        }

        pbrMetallicRoughness["metallicRoughnessTexture"] = metallicRoughnessTexture;

        if (!writeOcclusion) {
            // Do nothing
        } else {
            json occlusionTexture = json::object();
            occlusionTexture["index"] = index;

            material["occlusionTexture"] = occlusionTexture;
        }

        json texture = json::object();
        texture["source"] = index;
        textures.push_back(texture);

        json image = json::object();
        image["uri"] = imagePath;
        images.push_back(image);
    }

    if (writeNormal) {
        std::string imagePath = gltfPath + "/" + stem + "_normal";
        if (keepNormalImageData) {
            imagePath += normalImageRawExtension;

            if (!saveFile(normalImageRaw, imagePath)) {
                printf("Error: Could not save image raw '%s'\n", imagePath.c_str());

                return -1;
            }
        } else {
            imagePath += ".png";

            if (!stbi_write_png(imagePath.c_str(), normalImage.width, normalImage.height, normalImage.channels, normalImage.pixels.data(), 0)) {
                printf("Error: Could not save image '%s'\n", imagePath.c_str());

                return -1;
            }
        }

        size_t index = textures.size();

        json normalTexture = json::object();
        normalTexture["index"] = index;

        material["normalTexture"] = normalTexture;

        json texture = json::object();
        texture["source"] = index;
        textures.push_back(texture);

        json image = json::object();
        image["uri"] = imagePath;
        images.push_back(image);
    }

    if (writeEmissive) {
        std::string imagePath = gltfPath + "/" + stem + "_emissive";
        if (keepEmissiveImageData) {
            imagePath += emissiveImageRawExtension;

            if (!saveFile(emissiveImageRaw, imagePath)) {
                printf("Error: Could not save image raw '%s'\n", imagePath.c_str());

                return -1;
            }
        } else {
            imagePath += ".png";

            if (!stbi_write_png(imagePath.c_str(), emissiveImage.width, emissiveImage.height, emissiveImage.channels, emissiveImage.pixels.data(), 0)) {
                printf("Error: Could not save image '%s'\n", imagePath.c_str());

                return -1;
            }
        }

        size_t index = textures.size();

        json emissiveTexture = json::object();
        emissiveTexture["index"] = index;

        material["emissiveTexture"] = emissiveTexture;

        json emissiveFactor = json::array();
        emissiveFactor.push_back(1.0f);
        emissiveFactor.push_back(1.0f);
        emissiveFactor.push_back(1.0f);
        material["emissiveFactor"] = emissiveFactor;

        json texture = json::object();
        texture["source"] = index;
        textures.push_back(texture);

        json image = json::object();
        image["uri"] = imagePath;
        images.push_back(image);
    }

    material["name"] = stem;

    material["pbrMetallicRoughness"] = pbrMetallicRoughness;
    materials.push_back(material);
    glTF["materials"] = materials;

    if (textures.size() > 0) {
        glTF["textures"] = textures;
    }
    if (images.size() > 0) {
        glTF["images"] = images;
    }

    //

    std::string savename = gltfPath + "/" + stem + ".gltf";

    if (!saveFile(glTF.dump(3), savename)) {
        printf("Error: Could not save '%s'\n", savename.c_str());

        return -1;
    }

    printf("Success: Converted to '%s'\n", savename.c_str());

    return 0;
}