#include <iostream>
#include <cstdint>
#include <fstream>
#include <cmath>

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

namespace Filter
{
	void grayscale (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void sepia (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void reflection (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void blur (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
}

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

		Filter::blur(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
		std::cout << "Adding blur\n";

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

namespace Filter
{
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

	void sepia (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
	{
		for (int row {0}; row < height; ++row)
		{
			for (int col {0}; col < width; ++col)
			{
				double tB {static_cast<double>(pixelMatrix[row][col].blue)};
				double tG {static_cast<double>(pixelMatrix[row][col].green)};
				double tR {static_cast<double>(pixelMatrix[row][col].red)};

				double cB{tB};
				double cG{tG};
				double cR{tR};

				tB = round((0.272 * cR) + (0.534 * cG) + (0.131 * cB));
				tG = round((0.349 * cR) + (0.686 * cG) + (0.168 * cB));
				tR = round((0.393 * cR) + (0.769 * cG) + (0.189 * cB));

				if (tB > 255)
					tB = 255;

				if (tG > 255)
					tG = 255;

				if (tR > 255)
					tR = 255;

				pixelMatrix[row][col].blue = static_cast<int>(tB);
				pixelMatrix[row][col].green = static_cast<int>(tG);
				pixelMatrix[row][col].red = static_cast<int>(tR);

			}

		}
	}

	void reflection (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
	{
		for (int row {0}; row < height; ++row)
		{
			Pixel* rowData {new Pixel[width]};
			for (int col {0}; col < width; ++col)
			{
				rowData[width - col - 1].blue = pixelMatrix[row][col].blue;
				rowData[width - col - 1].green = pixelMatrix[row][col].green;
				rowData[width - col - 1].red = pixelMatrix[row][col].red;
			}

			for (int col {0}; col < width; ++col)
			{
				pixelMatrix[row][col].blue = rowData[col].blue;
				pixelMatrix[row][col].green = rowData[col].green;
				pixelMatrix[row][col].red = rowData[col].red;
			}

			delete[] rowData;
		}
	}

	void blur (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
	{
		int radius {20};
		int matrixWidth{(radius * 2) + 1};
		// int matrixElements{matrixWidth * matrixWidth};


		Pixel** copyPixelMatrix {new Pixel*[height]};
		for (int i{0}; i < height; ++i)
			copyPixelMatrix[i] = new Pixel[width];

		for (int row {0}; row < height; ++row)
		{
			for (int col {0}; col < width; ++col)
			{
				double sumBlue {0};
				double sumGreen {0};
				double sumRed{0};

				for (int rowS {row - radius}; rowS <= row + radius; ++rowS)
				{
					int rowSB {rowS};
					if (rowS < 0)
						continue;
					if (rowS >= height)
						continue;

					sumBlue += pixelMatrix[rowSB][col].blue;
					sumGreen += pixelMatrix[rowSB][col].green;
					sumRed += pixelMatrix[rowSB][col].red;
				}

				copyPixelMatrix[row][col].blue = static_cast<int>(round(sumBlue / matrixWidth));
				copyPixelMatrix[row][col].green = static_cast<int>(round(sumGreen / matrixWidth));
				copyPixelMatrix[row][col].red = static_cast<int>(round(sumRed / matrixWidth));
			}

		}

		for (int col {0}; col < width; ++col)
		{
			for (int row {0}; row < height; ++row)
			{
				double sumBlue {0};
				double sumGreen {0};
				double sumRed{0};

				for (int colS {col - radius}; colS <= col + radius; ++colS)
				{
					int colSB {colS};
					if (colS < 0)
						continue;
					if (colS >= width)
						continue;
					sumBlue += copyPixelMatrix[row][colSB].blue;
					sumGreen += copyPixelMatrix[row][colSB].green;
					sumRed += copyPixelMatrix[row][colSB].red;
				}

				copyPixelMatrix[row][col].blue = static_cast<int>(round(sumBlue / matrixWidth));
				copyPixelMatrix[row][col].green = static_cast<int>(round(sumGreen / matrixWidth));
				copyPixelMatrix[row][col].red = static_cast<int>(round(sumRed / matrixWidth));
			}
		}

		for (int row {0}; row < height; ++row)
		{
			for (int col {0}; col < width; ++col)
			{

				pixelMatrix[row][col].blue = copyPixelMatrix[row][col].blue;
				pixelMatrix[row][col].green = copyPixelMatrix[row][col].green;
				pixelMatrix[row][col].red = copyPixelMatrix[row][col].red;
			}

		}

		for(int i {0}; i < height; ++i)
			delete[] copyPixelMatrix[i];
		delete[] copyPixelMatrix;
	}
}


