#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

float clampf(float x, float minVal, float maxVal)
{
	float t = (x > minVal) ? x : minVal;

	return (t < maxVal) ? t : maxVal;
}

std::string toLowercase(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

void decomposePath(DecomposedPath& decomposedPath, const std::string& path)
{
	std::filesystem::path filesystemPath(path);

	decomposedPath.parentPath = filesystemPath.parent_path().generic_string();
	decomposedPath.stem = filesystemPath.stem().generic_string();
	decomposedPath.extension = filesystemPath.extension().generic_string();
}

bool loadImage(ImageDataResource& imageDataResource, const std::string& filename)
{
	DecomposedPath decomposedPath;
	decomposePath(decomposedPath, filename);

	int x = 0;
	int y = 0;
	int comp = 0;
	int req_comp = 0;

	uint8_t* tempData = static_cast<uint8_t*>(stbi_load(filename.c_str(), &x, &y, &comp, req_comp));
	if (!tempData)
	{
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
	if (!file.is_open())
	{
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
	if (!file.is_open())
	{
		return false;
	}

	file.write(output.data(), output.size());
	file.close();

	return true;
}

size_t gatherStem(const std::string& stem)
{
	size_t result = std::string::npos;

	result = stem.find("_Color");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Base_Color");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_basecolor");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Base Color");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_diffuse");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Opacity");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_opacity");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Metallic");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_metallic");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Roughness");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_roughness");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_rough");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_AO");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_ambientocclusion");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Normal");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_normal");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_nor");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_emissive");
	if (result != std::string::npos)
	{
		return result;
	}

	return result;
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("Usage: pbr2gltf2 folder [-m 1.0 -r 1.0 -n true -e true]\n");

		return 0;
	}

	//

	float defaultMetallicFactor = 1.0f;
	float defaultRoughnessFactor = 1.0f;
	bool keepNormalImageData = true;
	bool keepEmissiveImageData = true;

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-m") == 0 && (i + 1 < argc))
		{
			defaultMetallicFactor = clampf(std::stof(argv[i + 1]), 0.0f, 1.0f);
		}
		else if (strcmp(argv[i], "-r") == 0 && (i + 1 < argc))
		{
			defaultRoughnessFactor = clampf(std::stof(argv[i + 1]), 0.0f, 1.0f);
		}
		else if (strcmp(argv[i], "-n") == 0 && (i + 1 < argc))
		{
			if (strcmp(argv[i + 1], "true") == 0)
			{
				keepNormalImageData = true;
			}
			else if (strcmp(argv[i + 1], "false") == 0)
			{
				keepNormalImageData = false;
			}
		}
		else if (strcmp(argv[i], "-e") == 0 && (i + 1 < argc))
		{
			if (strcmp(argv[i + 1], "true") == 0)
			{
				keepEmissiveImageData = true;
			}
			else if (strcmp(argv[i + 1], "false") == 0)
			{
				keepEmissiveImageData = false;
			}
		}
	}

	//

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

	std::string path = argv[1];
    for (const auto& directoryEntry : std::filesystem::directory_iterator(path))
    {
    	std::string filename = directoryEntry.path().generic_string();

    	printf("Info: Processing '%s'\n", filename.c_str());

    	DecomposedPath decomposedPath;
    	decomposePath(decomposedPath, filename);

    	std::string lowercaseExtension = toLowercase(decomposedPath.extension);
    	if (!(lowercaseExtension == ".png" || lowercaseExtension == ".jpg" || lowercaseExtension == ".jpeg"))
    	{
    		continue;
    	}

    	ImageDataResource imageDataResource;
    	if (!loadImage(imageDataResource, filename))
    	{
    		printf("Warning: Skipping image '%s' because could not load size\n", filename.c_str());

    		continue;
    	}

    	if (init)
    	{
    		auto it = gatherStem(decomposedPath.stem);
    		if (it != std::string::npos)
    		{
    			stem = decomposedPath.stem.substr(0, it);
    		}

    		//

			baseColorImage.width = imageDataResource.width;
			baseColorImage.height = imageDataResource.height;
			baseColorImage.channels = 4;
			baseColorImage.pixels.resize(baseColorImage.channels * baseColorImage.width * baseColorImage.height);

			for (size_t y = 0; y < baseColorImage.height; y++)
			{
				for (size_t x = 0; x < baseColorImage.width; x++)
				{
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

			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 0] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 1] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 2] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 3] = 255;
				}
			}

			if (keepNormalImageData)
			{
				// Keeping original byte date
			}
			else
			{
				normalImage.width = imageDataResource.width;
				normalImage.height = imageDataResource.height;
				normalImage.channels = 3;
				normalImage.pixels.resize(normalImage.channels * normalImage.width * normalImage.height);

				// Not required for normal map
			}

			if (keepEmissiveImageData)
			{
				// Keeping original byte date
			}
			else
			{
				emissiveImage.width = imageDataResource.width;
				emissiveImage.height = imageDataResource.height;
				emissiveImage.channels = 3;
				emissiveImage.pixels.resize(emissiveImage.channels * emissiveImage.width * emissiveImage.height);

				// Not required for normal map
			}

			//

    		init = false;
    	}
    	else
    	{
    		if ((imageDataResource.width != baseColorImage.width) || (imageDataResource.height != baseColorImage.height))
    		{
    			printf("Warning: Skipping image '%s' because of size\n", filename.c_str());

    			continue;
    		}
    	}

    	//

    	bool hasBaseColor = ((filename.find("_Color.") != std::string::npos) || (filename.find("_Base_Color.") != std::string::npos) || (filename.find("_basecolor.") != std::string::npos) || (filename.find("_diffuse_") != std::string::npos) || (filename.find("_Base Color.") != std::string::npos));
    	if (hasBaseColor)
    	{
			for (size_t y = 0; y < baseColorImage.height; y++)
			{
				for (size_t x = 0; x < baseColorImage.width; x++)
				{
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 1];
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 2];
				}
			}

			printf("Info: Found base color\n");

			writeBaseColor = true;
    	}

    	bool hasOpacity = ((filename.find("_Opacity.") != std::string::npos) || (filename.find("_opacity.") != std::string::npos));
    	if (hasOpacity)
    	{
			for (size_t y = 0; y < baseColorImage.height; y++)
			{
				for (size_t x = 0; x < baseColorImage.width; x++)
				{
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels + x * baseColorImage.channels + 3] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
				}
			}

			printf("Info: Found alpha\n");

			writeOpacity = true;
    	}

    	bool hasMetallic = ((filename.find("_Metallic.") != std::string::npos) || (filename.find("_metallic.") != std::string::npos));
    	if (hasMetallic)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
				}
			}

			printf("Info: Found metallic\n");

			writeMetallic = true;
    	}

    	bool hasRoughness = ((filename.find("_Roughness.") != std::string::npos) || (filename.find("_roughness.") != std::string::npos) || (filename.find("_rough_") != std::string::npos));
    	if (hasRoughness)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
				}
			}

			printf("Info: Found roughness\n");

			writeRoughness = true;
    	}

    	bool hasOcclusion = ((filename.find("_AO.") != std::string::npos) || (filename.find("_ambientocclusion.") != std::string::npos) || (filename.find("_AO_") != std::string::npos));
    	if (hasOcclusion)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels + x * metallicRoughnessImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
				}
			}

			printf("Info: Found occlusion\n");

			writeOcclusion = true;
    	}

    	bool hasNormal = ((filename.find("_Normal.") != std::string::npos) || (filename.find("_normal.") != std::string::npos) || (filename.find("_nor_") != std::string::npos));
    	if (hasNormal)
    	{
    		if (keepNormalImageData)
    		{
    			if (!loadFile(normalImageRaw, filename))
    			{
    				printf("Error: Could not load image raw '%s'\n", filename.c_str());

    				return -1;
    			}

    			normalImageRawExtension = decomposedPath.extension;
    		}
    		else
    		{
    			for (size_t y = 0; y < normalImage.height; y++)
    			{
    				for (size_t x = 0; x < normalImage.width; x++)
    				{
    					normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 0];
    					normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 1];
    					normalImage.pixels.data()[y * normalImage.width * normalImage.channels + x * normalImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels + x * imageDataResource.channels + 2];
    				}
    			}
    		}

    		printf("Info: Found normal\n");

			writeNormal = true;
    	}

    	bool hasEmissive = (filename.find("_emissive.") != std::string::npos);
    	if (hasEmissive)
    	{
    		if (keepEmissiveImageData)
    		{
    			if (!loadFile(emissiveImageRaw, filename))
    			{
    				printf("Error: Could not load image raw '%s'\n", filename.c_str());

    				return -1;
    			}

    			emissiveImageRawExtension = decomposedPath.extension;
    		}
    		else
    		{
    			for (size_t y = 0; y < emissiveImage.height; y++)
    			{
    				for (size_t x = 0; x < emissiveImage.width; x++)
    				{
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

    if (writeBaseColor || writeOpacity)
    {
		std::string imagePath = stem + "_baseColor.png";
		if (!stbi_write_png(imagePath.c_str(), baseColorImage.width, baseColorImage.height, baseColorImage.channels, baseColorImage.pixels.data(), 0))
		{
			printf("Error: Could not save image '%s'\n", imagePath.c_str());

			return -1;
		}

		size_t index = textures.size();

		json baseColorTexture = json::object();
		baseColorTexture["index"] = index;

		pbrMetallicRoughness["baseColorTexture"] = baseColorTexture;

		if (!writeOpacity)
		{
			// Do nothing
		}
		else
		{
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

    if (writeMetallic || writeRoughness || writeOcclusion)
    {
		std::string imagePath = stem + "_metallicRoughness.png";
		if (!stbi_write_png(imagePath.c_str(), metallicRoughnessImage.width, metallicRoughnessImage.height, metallicRoughnessImage.channels, metallicRoughnessImage.pixels.data(), 0))
		{
			printf("Error: Could not save image '%s'\n", imagePath.c_str());

			return -1;
		}

		size_t index = textures.size();

		json metallicRoughnessTexture = json::object();
		metallicRoughnessTexture["index"] = index;

		if (!writeMetallic)
		{
			pbrMetallicRoughness["metallicFactor"] = defaultMetallicFactor;
		}
		if (!writeRoughness)
		{
			pbrMetallicRoughness["roughnessFactor"] = defaultRoughnessFactor;
		}

		pbrMetallicRoughness["metallicRoughnessTexture"] = metallicRoughnessTexture;

		if (!writeOcclusion)
		{
			// Do nothing
		}
		else
		{
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

    if (writeNormal)
    {
		std::string imagePath = stem + "_normal";
		if (keepNormalImageData)
		{
			imagePath += normalImageRawExtension;

			if (!saveFile(normalImageRaw, imagePath))
			{
				printf("Error: Could not save image raw '%s'\n", imagePath.c_str());

				return -1;
			}
		}
		else
		{
			imagePath += ".png";

			if (!stbi_write_png(imagePath.c_str(), normalImage.width, normalImage.height, normalImage.channels, normalImage.pixels.data(), 0))
			{
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

    if (writeEmissive)
    {
		std::string imagePath = stem + "_emissive";
		if (keepEmissiveImageData)
		{
			imagePath += emissiveImageRawExtension;

			if (!saveFile(emissiveImageRaw, imagePath))
			{
				printf("Error: Could not save image raw '%s'\n", imagePath.c_str());

				return -1;
			}
		}
		else
		{
			imagePath += ".png";

			if (!stbi_write_png(imagePath.c_str(), emissiveImage.width, emissiveImage.height, emissiveImage.channels, emissiveImage.pixels.data(), 0))
			{
				printf("Error: Could not save image '%s'\n", imagePath.c_str());

				return -1;
			}
		}

		size_t index = textures.size();

		json emissiveTexture = json::object();
		emissiveTexture["index"] = index;

		material["emissiveTexture"] = emissiveTexture;

		json emissiveFactor =  json::array();
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

	if (textures.size() > 0)
	{
		glTF["textures"] = textures;
	}
	if (images.size() > 0)
	{
		glTF["images"] = images;
	}

	//

    std::string savename = stem + ".gltf";

	if (!saveFile(glTF.dump(3), savename))
	{
		printf("Error: Could not save '%s'\n", savename.c_str());

		return -1;
	}

	printf("Success: Converted to '%s'\n", savename.c_str());

	return 0;
}
