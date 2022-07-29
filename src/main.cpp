#include <iostream>
#include <cstdint>

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
	std::int32_t bmpWidth;
};


int main()
{
	std::cout << "Hello World\n";

	return 0;
}


