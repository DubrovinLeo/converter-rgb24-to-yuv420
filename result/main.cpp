#include "yuv420.h"


int main()
{

	bitmap_image image("duck_rgb24.bmp");
	image.bgr_to_yuv420();
	//image.save("out.yuv420");

	yuv420 video("foreman_cif.yuv", 352, 288, 300);
	video.overlay(image, 10, 20);
	video.save("out.yuv");

	return 0;
}