#include <iostream>
#include <cstdint>

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


int main()
{
	std::cout << "Hello World\n";

	return 0;
}

