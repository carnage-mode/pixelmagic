#include <iostream>
#include <limits>
#include <cstdint>
#include <fstream>
#include <string>
#include <cmath>

// Defines a pixel
struct Pixel
{
	std::uint8_t red {};
	std::uint8_t green {};
	std::uint8_t blue {};
};

// Used for storing summed RGB values, hence the larger datatypes
struct SummedPixel
{
	int red {};
	int green {};
	int blue {};
};

// Helps us check the file format is correct. Must always be BM when converted
// to ASCII
using BITMAPFILEID = std::uint16_t;

// Contains information about the type, size, and layout of the image
struct BITMAPFILEHEADER
{
	std::uint32_t bmpSize; // Size of image. in bytes
	std::uint16_t bmpReserve1; // reserved, usually zero
	std::uint16_t bmpReserve2; // reserved, usually zero
	std::uint32_t bmpPixelArrayOffset; //offset to the bitmap bits (pixel array), in bytes
};

struct BITMAPINFOHEADER // contains metadata about the image
{
	std::uint32_t bmpDIBSize; // size of this struct in bytes, 40 is the only version we support
	std::int32_t  bmpWidth; // width of image in pixels
	std::int32_t  bmpHeight; // height of image in pixels
	std::uint16_t bmpColorPlanes; // must be 1,
	std::uint16_t bmpBitsPerPixel; // the number of bits in a pixel, we only support 24
	std::uint32_t bmpCompression; // type of compression, we only support 0 (no compression)
	std::uint32_t bmpImageSize; // since we dont use compression, this value is not relevant
	std::int32_t  bmpXPixelsPerMeter; // pixels per meter in x axis, used for printing
	std::int32_t  bmpYPixelsPerMeter; // pixels per meter in y axis, used for printing
	std::uint32_t bmpColorNum; // number of color indexes, we don't care about this in our implementation
	std::uint32_t bmpImportantColorNum; //number of important color indexes, same as above
};

//used for error handling
void ignoreLine();
void inputFailure();

//main menu
std::string mainMenu();

//All our filter functions live in their own namespace for organizational purposes
namespace Filter
{
	void revert (Pixel** pixelMatrix, Pixel** pixelMatrixOG, std::int32_t height, std::int32_t width);
	void grayscale (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void sepia (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void reflection (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void blur (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
	void edgeDetect (Pixel** pixelMatrix, std::int32_t height, std::int32_t width);
}

int main()
{
	while (true)
	{
		//stores the filename given by the user, calls the main menu function
		std::string fileName {mainMenu()};

		//opens th file in binary mode
		std::fstream imageFile(fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

		if (imageFile.fail())// continues main loop if the file fails to load
		{
			std::cout << '\n';
			std::cerr << fileName <<" can't be found. Try again\n";
			std::cout << '\n';
			continue;
		}
		else
			std::cout << "Found the file!\n";

		BITMAPFILEID id;
		BITMAPFILEHEADER fileheader;
		BITMAPINFOHEADER infoheader;

		//reads the id (magic number of the file)
		imageFile.read((char*)(&id), sizeof(id));

		if (id != 0x4D42) //checks if the first 2 bytes are equal to the string "BM"
		{
			std::cerr << "File type is not BMP!\n";
			imageFile.close(); //close the image
			continue;
		}

		//reads the metadata
		imageFile.read((char*)(&fileheader), sizeof(fileheader));
		imageFile.read((char*)(&infoheader), sizeof(infoheader));

		// Checks if the metadata uses 40 bytes,
		// other older and newer versions of bmp are not supported
		if (infoheader.bmpDIBSize != 40)
		{
			std::cerr << "Unsupported version of BMP\n";
			imageFile.close();
			continue;
		}

		// We only deal with 24 bit images
		if (infoheader.bmpBitsPerPixel != 24)
		{
			std::cerr << "Only 24 bit images supported\n";
			imageFile.close();
			continue;
		}

		// We dont deal with compressed images
		if (infoheader.bmpCompression != 0)
		{
			std::cerr << "Compressed images are not supported\n";
			imageFile.close();
			continue;
		}

		//inverts the height/width in case they are negative
		if (infoheader.bmpHeight < 0)
			infoheader.bmpHeight = -infoheader.bmpHeight;
		if (infoheader.bmpWidth < 0)
			infoheader.bmpWidth = -infoheader.bmpWidth;

		// Dynamically allocated 2D array that uses the pixel struct
		// used to store the bitmap (marix of pixels)
		Pixel** pixelMatrix {new Pixel*[infoheader.bmpHeight]};
		for (int i{0}; i < infoheader.bmpHeight; ++i)
			pixelMatrix[i] = new Pixel[infoheader.bmpWidth];

		// Create another dynamically allocated 2D array, used to store
		// a copy of the original image
		Pixel** pixelMatrixOG {new Pixel*[infoheader.bmpHeight]};
		for (int i{0}; i < infoheader.bmpHeight; ++i)
			pixelMatrixOG[i] = new Pixel[infoheader.bmpWidth];


		//Go to the exact byte were the pixel matrix (bitmap) starts
		imageFile.seekg(fileheader.bmpPixelArrayOffset);

		//copies the pixel matrix and stores it in memory for future manipultation
		for (int row {0}; row < infoheader.bmpHeight; ++row)
		{
			for (int col {0}; col < infoheader.bmpWidth; ++col)
			{
				pixelMatrix[row][col].blue = imageFile.get();
				pixelMatrixOG[row][col].blue = pixelMatrix[row][col].blue;

				pixelMatrix[row][col].green = imageFile.get();
				pixelMatrixOG[row][col].green = pixelMatrix[row][col].green;

				pixelMatrix[row][col].red = imageFile.get();
				pixelMatrixOG[row][col].red = pixelMatrix[row][col].red;
			}

			imageFile.seekg(infoheader.bmpWidth % 4, std::ios::cur);
		}

		bool edited {false}; //checks if the file is edited so as not to revert an unedited image and waste performance
		bool exit {false}; // checks if the user wants to exit from the filter menu

		//filter menu
		while (!exit)
		{
			std::cout << "1. Add Grayscale\n";
			std::cout << "2. Add Sepia\n";
			std::cout << "3. Reflect the image\n";
			std::cout << "4. Add blur\n";
			std::cout << "5. Detect edges\n";
			std::cout << "6. Revert image to original\n";
			std::cout << "7. Save and Exit\n";
			std::cout << "> ";
			int input {0};

			std::cin >> input;

			if (std::cin.fail())
			{
				inputFailure();
				continue;
			}
			else
				ignoreLine();

			switch(input)
			{
				case 1:
					std::cout << "Adding Grayscale...\n";
					Filter::grayscale(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = true;
					break;
				case 2:
					std::cout << "Adding Sepia...\n";
					Filter::sepia(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = true;
					break;
				case 3:
					std::cout << "Reflecting image...\n";
					Filter::reflection(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = true;
					break;
				case 4:
					std::cout << "Bluring the image...\n";
					Filter::blur(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = true;
					break;
				case 5:
					std::cout << "Adding edge detection...\n";
					Filter::edgeDetect(pixelMatrix, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = true;
					break;
				case 6:
					if (!edited)
					{
						std::cerr << "No changes applied. Cannot revert!\n\n";
						continue;
					}
					std::cout << "Reverting changes... \n";
					Filter::revert(pixelMatrix, pixelMatrixOG, infoheader.bmpHeight, infoheader.bmpWidth);
					edited = false;
					break;
				case 7:
					exit = true;
					continue;

				default:
					std::cerr << "\nInvalid input!\n\n";
					continue;
			}

			//Go to the exact byte were the pixel matrix (bitmap) starts
			imageFile.seekg(fileheader.bmpPixelArrayOffset);

			//Apply changes made (if any)
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
			std::cout << "Success\n\n";
		}

		std::cout << "\nGood bye!\n";
		imageFile.close(); //close the image after final edit

		//deallocate both pixel matrices
		for(int i {0}; i < infoheader.bmpHeight; ++i)
			delete[] pixelMatrix[i];
		delete[] pixelMatrix;

		for(int i {0}; i < infoheader.bmpHeight; ++i)
			delete[] pixelMatrixOG[i];
		delete[] pixelMatrixOG;

		break;
	}

	return 0;
}

// error handling
void ignoreLine()
{
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void inputFailure()
{
	std::cin.clear();
	ignoreLine();
	std::cerr << "\nInvalid input!\n\n";
}

//main menu
std::string mainMenu()
{
	std::string fileName {};

	while(true)
	{
		std::cout << "1. Add Image file\n";
		std::cout << "2. Exit\n";
		std::cout << "> ";
		int input {0};

		std::cin >> input;

		if (std::cin.fail())
		{
			inputFailure();
			continue;
		}
		else
			ignoreLine();

		switch(input)
		{
			case 1:
				std::cout << "Name of the image file (BMP format only): ";
				std::getline(std::cin >> std::ws, fileName);
				break;
			case 2:
				std::cout << "Good bye!\n";
				std::exit(0);
			default:
				std::cerr << "\nInvalid input!\n\n";
				continue;
		}
		break;
	}
	return fileName;
}

namespace Filter
{
	// reverts the pixelMatrix to it's original state
	void revert (Pixel** pixelMatrix, Pixel** pixelMatrixOG, std::int32_t height, std::int32_t width)
	{
		for (int row {0}; row < height; ++row)
		{
			for (int col {0}; col < width; ++col)
			{
				pixelMatrix[row][col].blue = pixelMatrixOG[row][col].blue;
				pixelMatrix[row][col].green = pixelMatrixOG[row][col].green;
				pixelMatrix[row][col].red = pixelMatrixOG[row][col].red;
			}

		}
	}

	// applies grayscale
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

	// applies sepia
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

	//reflects the image
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

	// blurs the image
	void blur (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
	{
		int radius {20};
		int matrixWidth {(2 * radius) + 1};
		int matrixArea {matrixWidth * matrixWidth};

		Pixel** copyPixelMatrix {new Pixel*[height]};
		for (int i{0}; i < height; ++i)
			copyPixelMatrix[i] = new Pixel[width];

		SummedPixel** sumtable {new SummedPixel*[height]};
		for (int i{0}; i < height; ++i)
			sumtable[i] = new SummedPixel[width];


		sumtable[0][0].blue = pixelMatrix[0][0].blue;
		sumtable[0][0].green = pixelMatrix[0][0].green;
		sumtable[0][0].red = pixelMatrix[0][0].red;


		for (int i{1}; i < height; ++i)
		{
			sumtable[i][0].blue = sumtable[i - 1][0].blue + pixelMatrix[i][0].blue;
			sumtable[i][0].green = sumtable[i - 1][0].green + pixelMatrix[i][0].green;
			sumtable[i][0].red = sumtable[i - 1][0].red + pixelMatrix[i][0].red;
		}

		for (int j{1}; j < width; ++j)
		{
			sumtable[0][j].blue = sumtable[0][j  - 1].blue + pixelMatrix[0][j].blue;
			sumtable[0][j].green = sumtable[0][j  - 1].green + pixelMatrix[0][j].green;
			sumtable[0][j].red = sumtable[0][j  - 1].red + pixelMatrix[0][j].red;
		}

		for (int i{1}; i < height; ++i)
			for (int j{1}; j < width; ++j)
			{
				sumtable[i][j].blue = pixelMatrix[i][j].blue + sumtable[i - 1][j].blue + sumtable[i][j - 1].blue - sumtable[i - 1][j - 1].blue;
				sumtable[i][j].green = pixelMatrix[i][j].green + sumtable[i - 1][j].green + sumtable[i][j - 1].green - sumtable[i - 1][j - 1].green;
				sumtable[i][j].red = pixelMatrix[i][j].red + sumtable[i - 1][j].red + sumtable[i][j - 1].red - sumtable[i - 1][j - 1].red;
			}

		for (int i{0}; i < height; ++i)
			for (int j{0}; j < width; ++j)
			{
				double avBlue{0};
				double avGreen{0};
				double avRed {0};

				int matrixAreaCopy {matrixArea};

				int iS{i + radius};
				int jS{j + radius};

				if (iS >= height)
				{
					iS = height - 1;
					matrixAreaCopy -= (radius - (iS - i)) * matrixWidth;
				}

				if (jS >= width)
				{
					jS = width - 1;
					matrixAreaCopy -= (radius - (jS - j)) * matrixWidth;
				}

				if ((i + radius) >= height && (j + radius) >= width)
					matrixAreaCopy += (radius - (iS - i)) * (radius - (jS - j));

				avBlue = sumtable[iS][jS].blue;
				avGreen = sumtable[iS][jS].green;
				avRed = sumtable[iS][jS].red;

				int iF{i + radius - matrixWidth};
				int jF{j + radius - matrixWidth};

				if (iF >=  0 && jF >=0)
				{
					avBlue -= sumtable[iF][jS].blue;
					avGreen -= sumtable[iF][jS].green;
					avRed -= sumtable[iF][jS].red;

					avBlue -= sumtable[iS][jF].blue;
					avGreen -= sumtable[iS][jF].green;
					avRed -= sumtable[iS][jF].red;

					avBlue += sumtable[iF][jF].blue;
					avGreen += sumtable[iF][jF].green;
					avRed += sumtable[iF][jF].red;
				}
				else if (iF >= 0)
				{
					avBlue -= sumtable[iF][jS].blue;
					avGreen -= sumtable[iF][jS].green;
					avRed -= sumtable[iF][jS].red;
				}
				else if (jF >= 0)
				{
					avBlue -= sumtable[iS][jF].blue;
					avGreen -= sumtable[iS][jF].green;
					avRed -= sumtable[iS][jF].red;
				}

				if (i - radius < 0)
					matrixAreaCopy -= (radius - i) * matrixWidth;

				if (j - radius < 0)
					matrixAreaCopy -= (radius - j) * matrixWidth;

				if((i - radius ) < 0 && (j - radius) < 0)
					matrixAreaCopy += (radius - i) * (radius - j);

				if((i - radius ) < 0 && (j + radius) >= width)
					matrixAreaCopy += (radius - i) * (radius - (jS - j));
				if((i + radius) >= height && (j - radius) < 0)
					matrixAreaCopy += (radius - (iS - i)) * (radius - j);

				copyPixelMatrix[i][j].blue = static_cast<int>(round(avBlue / matrixAreaCopy));
				copyPixelMatrix[i][j].green = static_cast<int>(round(avGreen / matrixAreaCopy));
				copyPixelMatrix[i][j].red = static_cast<int>(round(avRed / matrixAreaCopy));
			}

		for (int i{0}; i < height; ++i)
			for (int j{0}; j < width; ++j)
			{
				pixelMatrix [i][j].blue = copyPixelMatrix[i][j].blue;
				pixelMatrix [i][j].green = copyPixelMatrix[i][j].green;
				pixelMatrix [i][j].red = copyPixelMatrix[i][j].red;
			}

		for(int i {0}; i < height; ++i)
			delete[] copyPixelMatrix[i];
		delete[] copyPixelMatrix;

		for(int i {0}; i < height; ++i)
			delete[] sumtable[i];
		delete[] sumtable;
	}

	// Uses sobel operator to detect edges
	void edgeDetect (Pixel** pixelMatrix, std::int32_t height, std::int32_t width)
	{
		Pixel** copyPixelMatrix {new Pixel*[height + 2]};
		for (int i{0}; i < height + 2; ++i)
			copyPixelMatrix[i] = new Pixel[width + 2];

		for (int i{0}; i < height; ++i)
			for (int j{0}; j < width; ++j)
			{
				copyPixelMatrix[i + 1][j + 1].blue = pixelMatrix[i][j].blue;
				copyPixelMatrix[i + 1][j + 1].green = pixelMatrix[i][j].green;
				copyPixelMatrix[i + 1][j + 1].red = pixelMatrix[i][j].red;
			}

		grayscale(copyPixelMatrix, height, width);


		for (int i{1}; i < height + 1; ++i)
			for (int j{1}; j < width + 1; ++j)
			{
				int sum{0};
				double sumGy{0};
				double sumGx{0};

				sumGx -= copyPixelMatrix[i-1][j-1].red + (2 * copyPixelMatrix[i][j-1].red ) + copyPixelMatrix[i+1][j-1].red;
				sumGx += copyPixelMatrix[i-1][j+1].red  + (2 * copyPixelMatrix[i][j+1].red) + copyPixelMatrix[i+1][j+1].red;
				sumGx *= sumGx;

				sumGy -= copyPixelMatrix[i-1][j-1].red + (2 * copyPixelMatrix[i-1][j].red) + copyPixelMatrix[i-1][j+1].red;
				sumGy += copyPixelMatrix[i+1][j-1].red + (2 * copyPixelMatrix[i+1][j].red) + copyPixelMatrix[i+1][j+1].red;
				sumGy *= sumGy;

				sum = static_cast<int>(round(sqrt(sumGx + sumGy)));

				if (sum > 255)
					sum = 255;

				pixelMatrix[i - 1][j - 1].blue = sum;
				pixelMatrix[i - 1][j - 1].green = sum;
				pixelMatrix[i - 1][j - 1].red = sum;

			}


		for(int i {0}; i < height + 2; ++i)
			delete[] copyPixelMatrix[i];
		delete[] copyPixelMatrix;
	}
}


