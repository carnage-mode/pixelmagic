#include <iostream>
#include <cstdint>

struct pixel
{
	std::uint8_t red {};
	std::uint8_t green {};
	std::uint8_t blue {};
};

struct BITMAPFILEHEADER
{
	std::uint16_t bmpID;
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
	std::uint32_t biSizeImage;
	std::int32_t  biXPelsPerMeter;
	std::int32_t  biYPelsPerMeter;
	std::uint32_t biClrUsed;
	std::uint32_t biClrImportant;
};


int main()
{
	std::cout << "Hello World\n";

	return 0;
}


