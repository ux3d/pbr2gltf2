#ifndef PBR2GLTF2_H
#define PBR2GLTF2_H

#include <string>

namespace pbr2gltf2 {
int convert(const std::string& path, float defaultMetallicFactor, float defaultRoughnessFactor, bool keepNormalImageData, bool keepEmissiveImageData);
}

#endif // PBR2GLTF2_H