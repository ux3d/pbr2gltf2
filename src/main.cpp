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
	int req_comp = 4;

	uint8_t* tempData = static_cast<uint8_t*>(stbi_load(filename.c_str(), &x, &y, &comp, req_comp));
	if (!tempData)
	{
		return false;
	}

	imageDataResource.width = static_cast<uint32_t>(x);
	imageDataResource.height = static_cast<uint32_t>(y);
	imageDataResource.channels = 4;
	imageDataResource.pixels.resize(imageDataResource.width * imageDataResource.height * imageDataResource.channels);
	memcpy(imageDataResource.pixels.data(), tempData, imageDataResource.width * imageDataResource.height * imageDataResource.channels);

	free(tempData);

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

	result = stem.find("_Opacity");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Metallic");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Roughness");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_AO");
	if (result != std::string::npos)
	{
		return result;
	}

	result = stem.find("_Normal");
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
		printf("Usage: pbr2gltf2 folder\n");

		return 0;
	}

	std::string stem = "pbr";

	json glTF = json::object();

	json asset = json::object();
	asset["version"] = "2.0";
	asset["generator"] = "pbr2glTF by UX3D";

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

	ImageDataResource baseColorImage;

	ImageDataResource metallicRoughnessImage;

	ImageDataResource normalImage;

	std::string path = argv[1];
    for (const auto& directoryEntry : std::filesystem::directory_iterator(path))
    {
    	std::string filename = directoryEntry.path().generic_string();

    	printf("Info: Processing '%s'\n", filename.c_str());

    	DecomposedPath decomposedPath;
    	decomposePath(decomposedPath, filename);

    	ImageDataResource imageDataResource;
    	if (!loadImage(imageDataResource, filename))
    	{
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
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 0] = 255;
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 1] = 255;
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 2] = 255;
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 3] = 255;
				}
			}

			metallicRoughnessImage.width = imageDataResource.width;
			metallicRoughnessImage.height = imageDataResource.height;
			metallicRoughnessImage.channels = 4;
			metallicRoughnessImage.pixels.resize(metallicRoughnessImage.channels * metallicRoughnessImage.width * metallicRoughnessImage.height);

			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 0] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 1] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 2] = 255;
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 3] = 255;
				}
			}

			normalImage.width = imageDataResource.width;
			normalImage.height = imageDataResource.height;
			normalImage.channels = 3;
			normalImage.pixels.resize(normalImage.channels * normalImage.width * normalImage.height);

			// Not initialization required

    		init = false;
    	}
    	else
    	{
    		if ((imageDataResource.width != baseColorImage.width) || (imageDataResource.height != baseColorImage.height))
    		{
    			continue;
    		}
    	}

    	//

    	bool hasBaseColor = ((filename.find("_Color.") != std::string::npos) || (filename.find("_Base_Color.") != std::string::npos));
    	if (hasBaseColor)
    	{
			for (size_t y = 0; y < baseColorImage.height; y++)
			{
				for (size_t x = 0; x < baseColorImage.width; x++)
				{
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 1];
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 2];
				}
			}

			writeBaseColor = true;
    	}

    	bool hasOpacity = (filename.find("_Opacity.") != std::string::npos);
    	if (hasOpacity)
    	{
			for (size_t y = 0; y < baseColorImage.height; y++)
			{
				for (size_t x = 0; x < baseColorImage.width; x++)
				{
					baseColorImage.pixels.data()[y * baseColorImage.width * baseColorImage.channels  + x * baseColorImage.channels + 3] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
				}
			}

			material["alphaMode"] = "MASK";
			material["doubleSided"] = true;

			writeOpacity = true;
    	}

    	bool hasMetallic = (filename.find("_Metallic.") != std::string::npos);
    	if (hasMetallic)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
				}
			}

			writeMetallic = true;
    	}

    	bool hasRoughness = (filename.find("_Roughness.") != std::string::npos);
    	if (hasRoughness)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
				}
			}

			writeRoughness = true;
    	}

    	bool hasOcclusion = (filename.find("_AO.") != std::string::npos);
    	if (hasOcclusion)
    	{
			for (size_t y = 0; y < metallicRoughnessImage.height; y++)
			{
				for (size_t x = 0; x < metallicRoughnessImage.width; x++)
				{
					metallicRoughnessImage.pixels.data()[y * metallicRoughnessImage.width * metallicRoughnessImage.channels  + x * metallicRoughnessImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
				}
			}

			writeOcclusion = true;
    	}

    	bool hasNormal = (filename.find("_Normal.") != std::string::npos);
    	if (hasNormal)
    	{
			for (size_t y = 0; y < normalImage.height; y++)
			{
				for (size_t x = 0; x < normalImage.width; x++)
				{
					normalImage.pixels.data()[y * normalImage.width * normalImage.channels  + x * normalImage.channels + 0] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 0];
					normalImage.pixels.data()[y * normalImage.width * normalImage.channels  + x * normalImage.channels + 1] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 1];
					normalImage.pixels.data()[y * normalImage.width * normalImage.channels  + x * normalImage.channels + 2] = imageDataResource.pixels.data()[y * imageDataResource.width * imageDataResource.channels  + x * imageDataResource.channels + 2];
				}
			}

			writeNormal = true;
    	}
    }

    if (writeBaseColor || writeOpacity)
    {
		std::string imagePath = stem + "_baseColor.png";
		stbi_write_png(imagePath.c_str(), baseColorImage.width, baseColorImage.height, baseColorImage.channels, baseColorImage.pixels.data(), 0);

		size_t index = textures.size();

		json baseColorTexture = json::object();
		baseColorTexture["index"] = index;

		pbrMetallicRoughness["baseColorTexture"] = baseColorTexture;

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
		stbi_write_png(imagePath.c_str(), metallicRoughnessImage.width, metallicRoughnessImage.height, metallicRoughnessImage.channels, metallicRoughnessImage.pixels.data(), 0);

		size_t index = textures.size();

		json metallicRoughnessTexture = json::object();
		metallicRoughnessTexture["index"] = index;

		pbrMetallicRoughness["metallicRoughnessTexture"] = metallicRoughnessTexture;

		if (writeOcclusion)
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
		std::string imagePath = stem + "_normal.png";
		stbi_write_png(imagePath.c_str(), normalImage.width, normalImage.height, normalImage.channels, normalImage.pixels.data(), 0);

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
