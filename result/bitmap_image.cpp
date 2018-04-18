#include "bitmap_image.h"


bitmap_image::bitmap_image(const std::string& filename) :
	file_name_(filename),
	color_model_(rgb_model)
{
	load_bitmap();
}


unsigned int bitmap_image::bytes_per_pixel() const
{
	return bytes_per_pixel_;
}

unsigned int bitmap_image::pixel_count() const
{
	return width_ *  height_;
}

const BYTE* bitmap_image::data_ptr() const
{
	return data_.data();
}

unsigned int bitmap_image::width() const
{
	return width_;
}
unsigned int bitmap_image::height() const
{
	return height_;
}

unsigned char* bitmap_image::row(unsigned int row_index) const
{
	return const_cast<unsigned char*>(&data_[(row_index * row_increment_)]);
}

bitmap_image::color_models bitmap_image::color_model() const
{
	return color_model_;
}

void bitmap_image::load_bitmap()
{
	std::ifstream stream;
	try
	{
		stream.open(file_name_.c_str(), std::ios::binary);
		if (!stream)
		{
			throw std::invalid_argument("bitmap_image::load_bitmap() ERROR: bitmap_image - file "
				+ file_name_ + " not found!");
		}

		bfh_.clear();
		bih_.clear();
		bfh_.read(stream);
		bih_.read(stream);

		if (bfh_.type != 19778)
		{
			throw std::invalid_argument("bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid type value " 
				+ std::to_string(bfh_.type) + " expected 19778.");
		}

		if (bih_.bit_count != 24)
		{
			throw std::invalid_argument("bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid bit depth "
				+ std::to_string(bih_.bit_count) + " expected 24.");
		}

		if (bih_.size != bih_.struct_size())
		{
			throw std::invalid_argument("bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid BIH size "
				+ std::to_string(bih_.size) + " expected " + std::to_string(bih_.struct_size()));
		}

		width_ = bih_.width;
		height_ = bih_.height;
		bytes_per_pixel_ = bih_.bit_count >> 3;
		padding_ = (4 - ((3 * width_) % 4)) % 4;
		row_increment_ = width_ * bytes_per_pixel_;
		data_.resize(height_ * row_increment_);


		std::size_t bitmap_file_size = file_size(file_name_);

		std::size_t bitmap_logical_size = (height_ * width_ * bytes_per_pixel_) +
			(height_ * padding_) +
			bih_.struct_size() +
			bfh_.struct_size();

		if (bitmap_file_size != bitmap_logical_size)
		{
			throw std::invalid_argument("bitmap_image::load_bitmap() ERROR: bitmap_image - "
				 "Mismatch between logical and physical sizes of bitmap. Logical: " 
				+ std::to_string(bitmap_logical_size) + " Physical: " + std::to_string(bitmap_file_size));
		}

		char padding_data[4] = { 0,0,0,0 };

		for (unsigned int i = 0; i < height_; ++i)
		{
			unsigned char* data_ptr = row(height_ - i - 1); // read in inverted row order

			stream.read(reinterpret_cast<char*>(data_ptr), sizeof(char) * bytes_per_pixel_ * width_);
			stream.read(padding_data, padding_);
		}
		stream.close();
	} catch(const std::exception& e)
	{
		stream.close();
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}


void bitmap_image::bgr_to_yuv420()
{
	bih_.clear();
	bfh_.clear();
	color_model_ = yuv_420_model;

	std::vector<BYTE> y_component;
	y_component.reserve(pixel_count());
	std::vector<BYTE> u_component;
	u_component.reserve(pixel_count() / 4);
	std::vector<BYTE> v_component;
	v_component.reserve(pixel_count() / 4);

	for (size_t y = 0; y < height_; ++y)
	{
		for (size_t x = 0; x < (width_ * 3); x += 3)
		{
			const int b_addrass = y * width_ * 3 + x;
			int r = data_[b_addrass + 2];
			int g = data_[b_addrass + 1];
			int b = data_[b_addrass];

			y_component.push_back( ((66 * r + 129 * g + 25 * b) >> 8) + 16 );
		}
	}

	for (size_t y = 0; y < height_; y += 2)
	{
		for (size_t x = 0; x < (width_ * 3); x += 3)
		{
			const int b_addrass = y * width_ * 3 + x;
			int r = data_[b_addrass + 2];
			int g = data_[b_addrass + 1];
			int b = data_[b_addrass];

			if (x % 2 == 0)
			{
				u_component.push_back( ((-38 * r - 74 * g + 112 * b) >> 8) + 128 );
			}
			else
			{
				v_component.push_back( ((112 * r - 94 * g - 18 * b) >> 8) + 128 );
			}
		}
	}

	data_.resize(width_ * height_ * 3 / 2);
	data_.insert(data_.begin(), y_component.begin(), y_component.end());
	data_.insert(data_.begin() + pixel_count(), u_component.begin(), u_component.end());
	data_.insert(data_.begin() + pixel_count() + pixel_count() / 4, v_component.begin(), v_component.end());
	
	y_component.clear();
	u_component.clear();
	v_component.clear();
}


void bitmap_image::save(const std::string& file_name) const
{
	std::ofstream stream;
	try
	{
		stream.open(file_name.c_str(), std::ios::binary);

		if (!stream)
		{
			throw std::invalid_argument("bitmap_image::save_yuv(): Error - Could not open file "
				+ file_name + " for writing!");
		}

		switch (color_model_)
		{
			case yuv_420_model: {
				stream.write(reinterpret_cast<const char*>(data_.data()), width_ * height_ * 3 / 2);
				break;
			};
			case rgb_model: {
				bfh_.write(stream);
				bih_.write(stream);

				char padding_data[4] = { 0x00, 0x00, 0x00, 0x00 };

				for (unsigned int i = 0; i < height_; ++i)
				{
					const unsigned char* data_ptr = row(height_ - i - 1);
					stream.write(reinterpret_cast<const char*>(data_ptr), sizeof(unsigned char) * row_increment_);
					stream.write(padding_data, padding_);
				}
				break;
			};
			default:;
		}
		stream.close();
	}
	catch (const std::exception& e)
	{
		stream.close();
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	
}

std::size_t bitmap_image::file_size(const std::string& file_name) const
{
	std::ifstream file(file_name.c_str(), std::ios::in | std::ios::binary);
	if (!file) return 0;
	file.seekg(0, std::ios::end);
	return static_cast<std::size_t>(file.tellg());
}

template<typename T>
void bitmap_image::read_from_stream(std::ifstream& stream, T& t)
{
	stream.read(reinterpret_cast<char*>(&t), sizeof(T));
}

template <typename T>
void bitmap_image::write_to_stream(std::ofstream& stream, const T& t)
{
	stream.write(reinterpret_cast<const char*>(&t), sizeof(T));
}