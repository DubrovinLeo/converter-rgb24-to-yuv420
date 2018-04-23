#pragma once
#ifndef BITMAP_IMAGE_H
#define BITMAP_IMAGE_H

#include "utilities.h"

class bitmap_image
{
public:
	enum color_models
	{
		rgb_model = 0,
		yuv_420_model = 1
	};

	bitmap_image(const std::string& filename);

	unsigned int bytes_per_pixel() const;
	unsigned int pixel_count() const;
	const BYTE* data_ptr() const;
	unsigned int width() const;
	unsigned int height() const;
	color_models color_model() const;
	unsigned char* row(unsigned int row_index) const;
	void load_bitmap();
	void bgr_to_yuv420();
	void save(const std::string& file_name) const;

private:
	std::string  file_name_;
	unsigned int width_;
	unsigned int height_;
	unsigned int row_increment_;
	unsigned short bytes_per_pixel_;
	color_models color_model_;
	std::vector<BYTE> data_;
	unsigned int padding_;
	std::mutex mutex_;

	struct bitmap_file_header
	{
		unsigned short type;
		unsigned int   size;
		unsigned short reserved1;
		unsigned short reserved2;
		unsigned int   off_bits;

		unsigned int struct_size() const
		{
			return sizeof(type) +
				sizeof(size) +
				sizeof(reserved1) +
				sizeof(reserved2) +
				sizeof(off_bits);
		}

		void clear()
		{
			std::memset(this, 0x00, sizeof(bitmap_file_header));
		}
		void read(std::ifstream& stream)
		{
			// TODO: big endian
			read_from_stream(stream, type);
			read_from_stream(stream, size);
			read_from_stream(stream, reserved1);
			read_from_stream(stream, reserved2);
			read_from_stream(stream, off_bits);
		}
		void write(std::ofstream& stream) const
		{
			write_to_stream(stream, type);
			write_to_stream(stream, size);
			write_to_stream(stream, reserved1);
			write_to_stream(stream, reserved2);
			write_to_stream(stream, off_bits);
		}
	} bfh_;

	struct bitmap_information_header
	{
		unsigned int   size;
		unsigned int   width;
		unsigned int   height;
		unsigned short planes;
		unsigned short bit_count;
		unsigned int   compression;
		unsigned int   size_image;
		unsigned int   x_pels_per_meter;
		unsigned int   y_pels_per_meter;
		unsigned int   clr_used;
		unsigned int   clr_important;

		unsigned int struct_size() const
		{
			return sizeof(size) +
				sizeof(width) +
				sizeof(height) +
				sizeof(planes) +
				sizeof(bit_count) +
				sizeof(compression) +
				sizeof(size_image) +
				sizeof(x_pels_per_meter) +
				sizeof(y_pels_per_meter) +
				sizeof(clr_used) +
				sizeof(clr_important);
		}

		void clear()
		{
			std::memset(this, 0x00, sizeof(bitmap_information_header));
		}

		void read(std::ifstream& stream)
		{
			read_from_stream(stream, size);
			read_from_stream(stream, width);
			read_from_stream(stream, height);
			read_from_stream(stream, planes);
			read_from_stream(stream, bit_count);
			read_from_stream(stream, compression);
			read_from_stream(stream, size_image);
			read_from_stream(stream, x_pels_per_meter);
			read_from_stream(stream, y_pels_per_meter);
			read_from_stream(stream, clr_used);
			read_from_stream(stream, clr_important);
		}

		void write(std::ofstream& stream) const
		{
			write_to_stream(stream, size);
			write_to_stream(stream, width);
			write_to_stream(stream, height);
			write_to_stream(stream, planes);
			write_to_stream(stream, bit_count);
			write_to_stream(stream, compression);
			write_to_stream(stream, size_image);
			write_to_stream(stream, x_pels_per_meter);
			write_to_stream(stream, y_pels_per_meter);
			write_to_stream(stream, clr_used);
			write_to_stream(stream, clr_important);
		}
	} bih_;

	template<typename T>
	static void read_from_stream(std::ifstream& stream, T& t);

	template <typename T>
	static void write_to_stream(std::ofstream& stream, const T& t);

	static std::size_t file_size(const std::string& file_name);

	void compute_bgr_to_yuv420(std::vector<BYTE> &data, unsigned int offset_h, unsigned int length_h);
	std::tuple<BYTE, BYTE, BYTE> get_rgb(unsigned int offset) const;
};

#endif // BITMAP_IMAGE_H

