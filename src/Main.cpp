#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <regex>

#include <SFML/Graphics.hpp>

#include "TextureInfo.h"
#include "JsonConverter.h"
#include "Packer.h"
#include "ImageRenderer.h"

namespace fs = std::experimental::filesystem;

int main(int argc, const char** argv)
{
	if (argc < 2) return 0;

	std::string fileRegex = "";
	std::string folderPath = "";
	auto allowFlip = false;
	auto clearColor = sf::Color::Transparent;

	try
	{
		if (argc > 1) folderPath = argv[1];
		if (argc > 2) allowFlip = std::stoi(argv[2]) > 0 ? 1 : 0;
		if (argc > 3) fileRegex = argv[3];
		if (argc > 4)
		{
			unsigned int packed;
			std::stringstream ss;
			ss << std::hex << argv[4];
			ss >> packed;
			clearColor = sf::Color(packed);
		}
	}
	catch (std::string &exception)
	{
		std::cout << "invalid arguments" << std::endl << exception << std::endl;
		system("pause");
		return 0;
	}

	std::replace(folderPath.begin(), folderPath.end(), '\\', '/');
	auto userRegex = std::regex(fileRegex);
	auto fileNameRegex = std::regex("[a-zA-Z0-9-_\\{\\}\\(\\)\\[\\]\\.]+$"); //escaping regex escapes...
	auto textures = std::unordered_map<std::string, sf::Texture>();
	auto textureInfos = std::vector<TextureInfo>();

	for (auto &itr : fs::directory_iterator(folderPath))
	{
		const auto path = itr.path().string();

		std::smatch match;
		if (std::regex_search(path.begin(), path.end(), match, userRegex))
		{
			auto texture = sf::Texture();
			if (texture.loadFromFile(path))
			{
				std::regex_search(path.begin(), path.end(), match, fileNameRegex);
				auto info = TextureInfo{ Rectangle(texture.getSize().x, texture.getSize().y), match[0]};
				textures.emplace(match[0], texture);
				textureInfos.push_back(info);
			}
		}
	}

	auto bin = Packer::pack(textureInfos, allowFlip);
	JsonConverter::saveToJson(folderPath, bin);

	auto image = ImageRenderer::draw(bin, textures, clearColor);

	image.saveToFile(std::string(folderPath) + "/atlas.png");

	return 0;
}