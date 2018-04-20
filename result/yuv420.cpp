#include "yuv420.h";

yuv420::yuv420(const std::string& filename, unsigned int width, unsigned int height, unsigned int frames):
	width_(width),
	height_(height),
	number_frames_(frames)
{
	bytes_per_frame_ = width_ * height_ * 3 / 2;
	data_.resize(number_frames_);

	std::ifstream stream;
	try
	{
		stream.open(filename.c_str(), std::ios::binary);
		if(!stream.is_open())
		{
			throw std::invalid_argument("yuv420::load_data_from_stream() ERROR: stream not opened.");
		}
	
		for (int i = 0; i < number_frames_; i++)
		{
			data_[i].resize(bytes_per_frame_);
			stream.read(reinterpret_cast<char*>(&data_[i][0]), bytes_per_frame_);
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

void yuv420::overlay(const bitmap_image& image, const unsigned int offset_x, const unsigned int offset_y)
{
	try
	{
		if (image.color_model() != image.yuv_420_model)
		{
			throw std::invalid_argument("yuv420::overlay() ERROR: color model is not yuv420.");
		}

		if (image.width() + offset_x > width_ || image.height() + offset_y > height_)
		{
			throw std::invalid_argument("yuv420::overlay() ERROR: wrong size.");
		}

		const BYTE* U_source_offset = image.data_ptr() + image.pixel_count();
		const BYTE* V_source_offset = U_source_offset + (image.pixel_count() / 4);

		for (int frameN = 0; frameN < number_frames_; frameN++)
		{
			BYTE* frame = data_[frameN].data();
			BYTE* U_dest_offset = frame + width_ * height_;
			BYTE* V_dest_offset = U_dest_offset + width_ * height_ / 4;

			for (int line = 0; line < image.height(); line++)
			{
				const BYTE* Y_source = image.data_ptr() + line * image.width();
				BYTE* Y_dest = frame + ((offset_y + line) * width_ + offset_x);
				std::memcpy(Y_dest, Y_source, image.width());

				if (line % 2 == 0)
				{
					const unsigned int UV_source_step = line * image.width() / 4;
					const unsigned int UV_dest_step = (offset_y + line) * width_ / 4 + offset_x / 2;

					std::memcpy(U_dest_offset + UV_dest_step, U_source_offset + UV_source_step, image.width() / 2);
					std::memcpy(V_dest_offset + UV_dest_step, V_source_offset + UV_source_step, image.width() / 2);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

void yuv420::save(const std::string& filename) const
{
	std::ofstream stream;
	try
	{
		stream.open(filename.c_str(), std::ios::binary);
		if (!stream.is_open())
		{
			throw std::invalid_argument("yuv420::save() ERROR: stream not opened.");
		}

		for (int i = 0; i < number_frames_; i++)
		{
			std::vector<BYTE> frame = data_[i];
			stream.write(reinterpret_cast<const char*>(frame.data()), bytes_per_frame_);
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

