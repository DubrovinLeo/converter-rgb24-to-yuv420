#pragma once
#include "utilities.h"
#include "bitmap_image.h"

class yuv420
{
public:
	yuv420(const std::string& filename, unsigned int width, unsigned int height, unsigned int frames);
	//void load_data();
	//void overlay(const bitmap_image& image, uint32_t offset_x, uint32_t offset_y);
	void overlay(const bitmap_image& image, const uint32_t offset_x = 0, const uint32_t offset_y = 0);
	void save(const std::string& filename) const;
private:
	uint32_t width_;
	uint32_t height_;
	uint32_t number_frames_;
	uint32_t bytes_per_frame_;
	std::vector<std::vector<BYTE>> data_;
};