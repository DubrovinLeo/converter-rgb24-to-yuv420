#pragma once
#include "utilities.h"
#include "bitmap_image.h"

class yuv420
{
public:
	yuv420(const std::string& filename, unsigned int width, unsigned int height, unsigned int frames);
	void overlay(const bitmap_image& image, const uint32_t offset_x = 0, const uint32_t offset_y = 0);
	void save(const std::string& filename) const;
private:
	unsigned int width_;
	unsigned int height_;
	unsigned int number_frames_;
	unsigned int bytes_per_frame_;
	std::vector<std::vector<BYTE>> data_;
};