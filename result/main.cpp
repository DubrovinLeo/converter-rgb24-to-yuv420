#include "yuv420.h"

int main()
{
	bitmap_image image("res/duck_rgb24.bmp");
	image.bgr_to_yuv420();
	image.save("res/out_img.yuv");
	
	yuv420 video("res/foreman_cif.yuv", 352, 288, 300);
	video.overlay(image, 10, 20);
	video.save("res/out_video.yuv");

	return 0;
}