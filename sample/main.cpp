#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <modplay.h>

std::vector<uint8_t> load_file(const char* filename);

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		std::cout << "Usage: this.exe file_to_play" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<uint8_t> bytes = load_file(argv[1]);
	if(bytes.empty())
	{
		std::cout << "File " << argv[1] << " is missing or empty!" << std::endl;
		return EXIT_FAILURE;
	}

	ModPlay* Music = new ModPlay(bytes.data(), bytes.size());
	Music->play();

	std::cout << "Press any key to stop..." << std::endl;
	char dummy;
	std::cin >> dummy;

	Music->pause();
	delete Music;

	return EXIT_SUCCESS;
}

std::vector<uint8_t> load_file(const char* filename)
{
	std::vector<uint8_t> bytes;

	std::ifstream file(filename, std::ios::in | std::ios::binary);
	if(file)
	{
		size_t filesize = file.seekg(0, std::ios::end).tellg();
		file.seekg(0);
		bytes.resize(filesize);
		file.read(reinterpret_cast<char*>(&bytes[0]), filesize);
	}

	return bytes;
}
