#include <iostream>
#include <cstdint>
#include <fstream>

struct Pixel
{
	std::uint8_t red {};
	std::uint8_t green {};
	std::uint8_t blue {};
};

using BITMAPFILEID = std::uint16_t; //Helps us check the file format is correct

struct BITMAPFILEHEADER //
{
	std::uint32_t bmpSize;
	std::uint16_t bmpReserve1;
	std::uint16_t bmpReserve2;
	std::uint32_t bmpPixelArrayOffset;
};

struct BITMAPINFOHEADER
{
	std::uint32_t bmpDIBSize;
	std::int32_t  bmpWidth;
	std::int32_t  bmpHeight;
	std::uint16_t bmpColorPlanes;
	std::uint16_t bmpBitsPerPixel;
	std::uint32_t bmpCompression;
	std::uint32_t bmpImageSize;
	std::int32_t  bmpXPixelsPerMeter;
	std::int32_t  bmpYPixelsPerMeter;
	std::uint32_t bmpColorNum;
	std::uint32_t bmpImportantColorNum;
};

void grayscale (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);

int main()
{
	while (true)
	{
		std::cout << "Name of the image file (BMP format only): ";
		std::string fileName {};
		std::getline(std::cin >> std::ws, fileName);
		std::fstream imageFile(fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

		if (imageFile.fail())
		{
			std::cerr << fileName <<" can't be found. Try again\n";
			continue;
		}
		else
			std::cout << "Found the file!\n";

		BITMAPFILEID id;
		imageFile.read((char*)(&id), sizeof(id));

		if (id != 0x4D42)
		{
			std::cerr << "File type is not BMP!\n";
			imageFile.close();
			continue;
		}

		BITMAPFILEHEADER fileheader;
		BITMAPINFOHEADER infoheader;

		imageFile.read((char*)(&fileheader), sizeof(fileheader));
		imageFile.read((char*)(&infoheader), sizeof(infoheader));

		if (infoheader.bmpDIBSize != 40)
		{
			std::cerr << "Unsupported version of BMP\n";
			imageFile.close();
			continue;
		}

		if (infoheader.bmpBitsPerPixel != 24)
		{
			std::cerr << "Only 24 bit images supported\n";
			imageFile.close();
			continue;
		}

		if (infoheader.bmpCompression != 0)
		{
			std::cerr << "Compressed images are not supported\n";
			imageFile.close();
			continue;
		}

		Pixel** pixelMatrix {new Pixel*[infoheader.bmpHeight]};
		for (int i{0}; i < infoheader.bmpHeight; ++i)
			pixelMatrix[i] = new Pixel[infoheader.bmpWidth];

		if (infoheader.bmpHeight < 0)
			infoheader.bmpHeight = -infoheader.bmpHeight;

		imageFile.seekg(fileheader.bmpPixelArrayOffset);

		for (int row {0}; row < infoheader.bmpHeight; ++row)
		{
			for (int col {0}; col < infoheader.bmpWidth; ++col)
			{
				pixelMatrix[row][col].blue = imageFile.get();
				pixelMatrix[row][col].green = imageFile.get();
				pixelMatrix[row][col].red = imageFile.get();
			}

			imageFile.seekg(infoheader.bmpWidth % 4, std::ios::cur);
		}

		grayscale(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
		std::cout << "Adding grayscale\n";

		imageFile.seekg(fileheader.bmpPixelArrayOffset);

		for (int row {0}; row < infoheader.bmpHeight; ++row)
		{
			for (int col {0}; col < infoheader.bmpWidth; ++col)
			{
				imageFile.put(pixelMatrix[row][col].blue);
				imageFile.put(pixelMatrix[row][col].green);
				imageFile.put(pixelMatrix[row][col].red);
			}

			imageFile.seekg(infoheader.bmpWidth % 4, std::ios::cur);
		}
		std::cout << "Success\n";
		imageFile.close();

		for(int i {0}; i < infoheader.bmpHeight; ++i)
			delete[] pixelMatrix[i];
		delete[] pixelMatrix;

		break;
	}

	return 0;
}

void grayscale (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
{
	for (int row {0}; row < height; ++row)
	{
		for (int col {0}; col < width; ++col)
		{
			int average {pixelMatrix[row][col].blue + pixelMatrix[row][col].green + pixelMatrix[row][col].red};
			average /= 3;
			pixelMatrix[row][col].blue = average;
			pixelMatrix[row][col].green = average;
			pixelMatrix[row][col].red = average;
		}

	}
}
