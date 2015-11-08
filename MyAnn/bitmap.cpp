#include "bitmap.h"


bitmap::bitmap(): fileName(""),data (0),length(0),width (0),height(0),rowIncrement(0),bytesPerPixel(3),channel_arrangement_(bgr_mode)
{

}

bitmap::bitmap(const std::string& filename)	: fileName(filename),data(0),length(0),	width (0),height(0),rowIncrement(0),bytesPerPixel(0),channel_arrangement_(bgr_mode)
{
	load_bitmap();
}

bitmap::bitmap(const unsigned int _width, const unsigned int _height)
	: fileName(""),
	data  (0),
	length(0),
	width(_width),
	height(_height),
	rowIncrement(0),
	bytesPerPixel(3),
	channel_arrangement_(bgr_mode)
{
	create_bitmap();
}

/*
*	Only creates view of bitmap. For deep copy use deepCopy().
*/
bitmap::bitmap(const bitmap& image)	: fileName(image.fileName),	data(0),width(image.width),	height(image.height),rowIncrement(0),bytesPerPixel(3),channel_arrangement_(bgr_mode)
{
	create_bitmap();

	std::copy(image.data.get(), image.data.get() + image.length, data.get());
}

bitmap::~bitmap()
{

}
Matrix bitmap::getMatrix()
{
	Matrix output(height,width,3);
	for (size_t i = 0; i < height*width*3; i++)
	{
		output.getData()[i]=(float)data.get()[i] / 255.0f;
	}
	return output;
}

void bitmap::setMatrix(Matrix in)
{
	width=in.cols;
	height=in.rows;
	rowIncrement=0;
	bytesPerPixel=3;
	channel_arrangement_=channel_arrangement::bgr_mode;

	create_bitmap();
	for (size_t i = 0; i < length; i++)
	{
		data.get()[i]=(unsigned char)(in.getData()[i]*255.0f);
	}
}
/*
*	Only creates view of bitmap. For deep copy use deepCopy().
*/
bitmap& bitmap::operator=(const bitmap& image)
{
	if (data != image.data)
	{
		fileName       = image.fileName;
		bytesPerPixel = image.bytesPerPixel;
		width           = image.width;
		height          = image.height;
		rowIncrement   = 0;
		channel_arrangement_    = image.channel_arrangement_;
		create_bitmap();
		data=image.data;
		//std::copy(image.data, image.data + image.length, data);
	}
	return *this;
}

bool bitmap::operator!()
{
	return (data == 0) ||(length == 0) ||(width == 0) ||(height == 0) ||(rowIncrement == 0);
}

void bitmap::clear(const unsigned char v)
{
	std::fill(data.get(),data.get() + length,v);
}

unsigned char bitmap::red_channel(const unsigned int x, const unsigned int y) const
{
	return data.get()[(y * rowIncrement) + (x * bytesPerPixel + 2)];
}

unsigned char bitmap::green_channel(const unsigned int x, const unsigned int y) const
{
	return data.get()[(y * rowIncrement) + (x * bytesPerPixel + 1)];
}

unsigned char bitmap::blue_channel (const unsigned int x, const unsigned int y) const
{
	return data.get()[(y * rowIncrement) + (x * bytesPerPixel + 0)];
}

void bitmap::red_channel(const unsigned int x, const unsigned int y, const unsigned char value)
{
	data.get()[(y * rowIncrement) + (x * bytesPerPixel + 2)] = value;
}

void bitmap::green_channel(const unsigned int x, const unsigned int y, const unsigned char value)
{
	data.get()[(y * rowIncrement) + (x * bytesPerPixel + 1)] = value;
}

void bitmap::blue_channel (const unsigned int x, const unsigned int y, const unsigned char value)
{
	data.get()[(y * rowIncrement) + (x * bytesPerPixel + 0)] = value;
}

unsigned char* bitmap::row(unsigned int row_index) const
{
	return data.get() + (row_index * rowIncrement);
}

void bitmap::get_pixel(const unsigned int x, const unsigned int y,unsigned char& red,unsigned char& green,unsigned char& blue)
{
	const unsigned int y_offset = y * rowIncrement;
	const unsigned int x_offset = x * bytesPerPixel;
	blue  = data.get()[y_offset + x_offset + 0];
	green = data.get()[y_offset + x_offset + 1];
	red   = data.get()[y_offset + x_offset + 2];
}

void bitmap::set_pixel(const unsigned int x, const unsigned int y,const unsigned char red,const unsigned char green,const unsigned char blue)
{
	const unsigned int y_offset = y * rowIncrement;
	const unsigned int x_offset = x * bytesPerPixel;
	data.get()[y_offset + x_offset + 0] = blue;
	data.get()[y_offset + x_offset + 1] = green;
	data.get()[y_offset + x_offset + 2] = red;
}

bool bitmap::copy_from(const bitmap& image)
{
	if (
		(image.height != height) ||
		(image.width  != width )
		)
	{
		return false;
	}

	std::copy(image.data.get(),image.data.get() + image.length,data.get());
	return true;
}

bool bitmap::copy_from(const bitmap& source_image,
					   const unsigned int& x_offset,
					   const unsigned int& y_offset)
{
	if ((x_offset + source_image.width ) > width ) { return false; }
	if ((y_offset + source_image.height) > height) { return false; }

	for (unsigned int y = 0; y < source_image.height; ++y)
	{
		unsigned char* itr1           = row(y + y_offset) + x_offset * bytesPerPixel;
		const unsigned char* itr2     = source_image.row(y);
		const unsigned char* itr2_end = itr2 + source_image.width * bytesPerPixel;
		std::copy(itr2,itr2_end,itr1);
	}
	return true;
}

bool bitmap::region(const unsigned int& x,
					const unsigned int& y,
					const unsigned int& _width,
					const unsigned int& _height,
					bitmap& dest_image)
{
	if ((x + _width ) > width ) { return false; }
	if ((y + _height) > height) { return false; }

	if (
		(dest_image.width  < width ) ||
		(dest_image.height < height)
		)
	{
		dest_image.setwidth_height(_width,_height);
	}

	for (unsigned int r = 0; r < _height; ++r)
	{
		unsigned char* itr1     = row(r + y) + x * bytesPerPixel;
		unsigned char* itr1_end = itr1 + (_width * bytesPerPixel);
		unsigned char* itr2     = dest_image.row(r);
		std::copy(itr1,itr1_end,itr2);
	}

	return true;
}

bool bitmap::set_region(const unsigned int& x,
						const unsigned int& y,
						const unsigned int& _width,
						const unsigned int& _height,
						const unsigned char& value)
{
	if ((x + _width) > width)   { return false; }
	if ((y + _height) > height) { return false; }

	for (unsigned int r = 0; r < _height; ++r)
	{
		unsigned char* itr     = row(r + y) + x * bytesPerPixel;
		unsigned char* itr_end = itr + (_width * bytesPerPixel);
		std::fill(itr,itr_end,value);
	}

	return true;
}

bool bitmap::set_region(const unsigned int& x,
						const unsigned int& y,
						const unsigned int& _width,
						const unsigned int& _height,
						const color_plane color,
						const unsigned char& value)
{
	if ((x + _width) > width)   { return false; }
	if ((y + _height) > height) { return false; }

	const unsigned int color_plane_offset = offset(color);

	for (unsigned int r = 0; r < _height; ++r)
	{
		unsigned char* itr     = row(r + y) + x * bytesPerPixel + color_plane_offset;
		unsigned char* itr_end = itr + (_width * bytesPerPixel);

		while (itr != itr_end)
		{
			*itr  = value;
			itr += bytesPerPixel;
		}
	}

	return true;
}

bool bitmap::set_region(const unsigned int& x,
						const unsigned int& y,
						const unsigned int& _width,
						const unsigned int& _height,
						const unsigned char& red,
						const unsigned char& green,
						const unsigned char& blue)
{
	if ((x +  _width) >  width) { return false; }
	if ((y + _height) > height) { return false; }

	for (unsigned int r = 0; r < _height; ++r)
	{
		unsigned char* itr     = row(r + y) + x * bytesPerPixel;
		unsigned char* itr_end = itr + (_width * bytesPerPixel);

		while (itr != itr_end)
		{
			*(itr++) = blue;
			*(itr++) = green;
			*(itr++) = red;
		}
	}

	return true;
}

void bitmap::reflective_image(bitmap& image)
{
	image.setwidth_height(3 * width, 3 * height,true);
	image.copy_from(*this,width,height);
	vertical_flip();
	image.copy_from(*this,width,0);
	image.copy_from(*this,width,2 * height);
	vertical_flip();
	horizontal_flip();
	image.copy_from(*this,0,height);
	image.copy_from(*this,2 * width,height);
	horizontal_flip();
}

unsigned int bitmap::_width() const
{
	return width;
}

unsigned int bitmap::_height() const
{
	return height;
}

unsigned int bitmap::bytes_per_pixel() const
{
	return bytesPerPixel;
}

unsigned int bitmap::pixel_count() const
{
	return width *  height;
}

void bitmap::setwidth_height(const unsigned int _width,
							 const unsigned int _height,
							 const bool clear)
{
	width  = _width;
	height = _height;

	create_bitmap();

	if (clear)
	{
		std::fill(data.get(),data.get() + length,0x00);
	}
}

void bitmap::save_image(const std::string& _fileName)
{
	std::ofstream stream(_fileName.c_str(),std::ios::binary);

	if (!stream)
	{
		std::cout << "bitmap::save_image(): Error - Could not open file "  << _fileName << " for writing!" << std::endl;
		return;
	}

	bitmap_file_header bfh;
	bitmap_information_header bih;

	bih._width            = width;
	bih._height           = height;
	bih.bit_count        = static_cast<unsigned short>(bytesPerPixel << 3);
	bih.clr_important    =  0;
	bih.clr_used         =  0;
	bih.compression      =  0;
	bih.planes           =  1;
	bih.size             = 40;
	bih.x_pels_per_meter =  0;
	bih.y_pels_per_meter =  0;
	bih.size_image       = (((bih._width * bytesPerPixel) + 3) & 0x0000FFFC) * bih._height;

	bfh.type      = 19778;
	bfh.size      = 55 + bih.size_image;
	bfh.reserved1 = 0;
	bfh.reserved2 = 0;
	bfh.off_bits  = bih.struct_size() + bfh.struct_size();

	write_bfh(stream,bfh);
	write_bih(stream,bih);

	unsigned int padding = (4 - ((3 * width) % 4)) % 4;
	char padding_data[4] = {0x0,0x0,0x0,0x0};

	for (unsigned int i = 0; i < height; ++i)
	{
		unsigned char* data_ptr = data.get() + (rowIncrement * (height - i - 1));
		stream.write(reinterpret_cast<char*>(data_ptr),sizeof(unsigned char) * bytesPerPixel * width);
		stream.write(padding_data,padding);
	}

	stream.close();
}

void bitmap::set_all_ith_bits_low(const unsigned int bitr_index)
{
	unsigned char mask = static_cast<unsigned char>(~(1 << bitr_index));

	for (unsigned char* itr = data.get(); itr != data.get() + length; ++itr)
	{
		*itr &= mask;
	}
}

void bitmap::set_all_ith_bits_high(const unsigned int bitr_index)
{
	unsigned char mask = static_cast<unsigned char>(1 << bitr_index);

	for (unsigned char* itr = data.get(); itr != data.get() + length; ++itr)
	{
		*itr |= mask;
	}
}

void bitmap::set_all_ith_channels(const unsigned int& channel, const unsigned char& value)
{
	for (unsigned char* itr = (data.get() + channel); itr < (data.get() + length); itr += bytesPerPixel)
	{
		*itr = value;
	}
}

void bitmap::set_channel(const color_plane color,const unsigned char& value)
{
	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); itr += bytesPerPixel)
	{
		*itr = value;
	}
}

void bitmap::ror_channel(const color_plane color, const unsigned int& ror)
{
	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); itr += bytesPerPixel)
	{
		*itr = static_cast<unsigned char>(((*itr) >> ror) | ((*itr) << (8 - ror)));
	}
}

void bitmap::set_all_channels(const unsigned char& value)
{
	for (unsigned char* itr = data.get(); itr < (data.get() + length); )
	{
		*(itr++) = value;
	}
}

void bitmap::set_all_channels(const unsigned char& r_value,
							  const unsigned char& g_value,
							  const unsigned char& b_value)
{
	for (unsigned char* itr = (data.get() + 0); itr < (data.get() + length); itr += bytesPerPixel)
	{
		*(itr + 0) = b_value;
		*(itr + 1) = g_value;
		*(itr + 2) = r_value;
	}
}

void bitmap::invert_color_planes()
{
	for (unsigned char* itr = data.get(); itr < (data.get() + length); *itr = ~(*itr), ++itr);
}

void bitmap::add_to_color_plane(const color_plane color,const unsigned char& value)
{
	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); (*itr) += value, itr += bytesPerPixel);
}

void bitmap::convert_to_grayscale()
{
	double r_scaler = 0.299;
	double g_scaler = 0.587;
	double b_scaler = 0.114;

	if (rgb_mode == channel_arrangement_)
	{
		double tmp = r_scaler;
		r_scaler = b_scaler;
		b_scaler = tmp;
	}

	for (unsigned char* itr = data.get(); itr < (data.get() + length); )
	{
		unsigned char gray_value = static_cast<unsigned char>((r_scaler * (*(itr + 2))) +
			(g_scaler * (*(itr + 1))) +
			(b_scaler * (*(itr + 0))) );
		*(itr++) = gray_value;
		*(itr++) = gray_value;
		*(itr++) = gray_value;
	}
}

const unsigned char* bitmap::getData()
{
	return data.get();
}

void bitmap::bgr_to_rgb()
{
	if ((bgr_mode == channel_arrangement_) && (3 == bytesPerPixel))
	{
		reverse_channels();
		channel_arrangement_ = rgb_mode;
	}
}

void bitmap::rgb_to_bgr()
{
	if ((rgb_mode == channel_arrangement_) && (3 == bytesPerPixel))
	{
		reverse_channels();
		channel_arrangement_ = bgr_mode;
	}
}

void bitmap::reverse()
{
	unsigned char* itr1 = data.get();
	unsigned char* itr2 = (data.get() + length) - bytesPerPixel;

	while (itr1 < itr2)
	{
		for (std::size_t i = 0; i < bytesPerPixel; ++i)
		{
			unsigned char* citr1 = itr1 + i;
			unsigned char* citr2 = itr2 + i;
			unsigned char tmp = *citr1;
			*citr1 = *citr2;
			*citr2 = tmp;
		}

		itr1 += bytesPerPixel;
		itr2 -= bytesPerPixel;
	}
}

void bitmap::horizontal_flip()
{
	for (unsigned int y = 0; y < height; ++y)
	{
		unsigned char* itr1 = row(y);
		unsigned char* itr2 = itr1 + rowIncrement - bytesPerPixel;

		while (itr1 < itr2)
		{
			for (unsigned int i = 0; i < bytesPerPixel; ++i)
			{
				unsigned char* p1 = (itr1 + i);
				unsigned char* p2 = (itr2 + i);
				unsigned char tmp = *p1;
				*p1 = *p2;
				*p2 = tmp;
			}

			itr1 += bytesPerPixel;
			itr2 -= bytesPerPixel;
		}
	}
}

void bitmap::vertical_flip()
{
	for (unsigned int y = 0; y < (height / 2); ++y)
	{
		unsigned char* itr1 = row(y);
		unsigned char* itr2 = row(height - y - 1);

		for (std::size_t x = 0; x < rowIncrement; ++x)
		{
			unsigned char tmp = *(itr1 + x);
			*(itr1 + x) = *(itr2 + x);
			*(itr2 + x) = tmp;
		}
	}
}

void bitmap::export_color_plane(const color_plane color, unsigned char* image)
{
	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); ++image, itr += bytesPerPixel)
	{
		(*image) = (*itr);
	}
}

void bitmap::export_color_plane(const color_plane color, bitmap& image)
{
	if (
		(width  != image.width ) ||
		(height != image.height)
		)
	{
		image.setwidth_height(width,height);
	}

	image.clear();

	unsigned char* itr1     = (data.get() + offset(color));
	unsigned char* itr1_end = (data.get() + length);
	unsigned char* itr2     = (image.data.get() + offset(color));

	while (itr1 < itr1_end)
	{
		(*itr2) = (*itr1);
		itr1 += bytesPerPixel;
		itr2 += bytesPerPixel;
	}
}

void bitmap::export_response_image(const color_plane color, double* response_image)
{
	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); ++response_image, itr += bytesPerPixel)
	{
		(*response_image) = (1.0 * (*itr)) / 256.0;
	}
}

void bitmap::export_gray_scale_response_image(double* response_image)
{
	for (unsigned char* itr = data.get(); itr < (data.get() + length); itr += bytesPerPixel)
	{
		unsigned char gray_value = static_cast<unsigned char>((0.299 * (*(itr + 2))) +
			(0.587 * (*(itr + 1))) +
			(0.114 * (*(itr + 0))));
		(*response_image) = (1.0 * gray_value) / 256.0;
	}
}

void bitmap::export_rgb(double* red, double* green, double* blue) const
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		(*blue)  = (1.0 * (*(itr++))) / 256.0;
		(*green) = (1.0 * (*(itr++))) / 256.0;
		(*red)   = (1.0 * (*(itr++))) / 256.0;
	}
}

void bitmap::export_rgb(float* red, float* green, float* blue) const
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		(*blue)  = (1.0f * (*(itr++))) / 256.0f;
		(*green) = (1.0f * (*(itr++))) / 256.0f;
		(*red)   = (1.0f * (*(itr++))) / 256.0f;
	}
}

void bitmap::export_rgb(unsigned char* red, unsigned char* green, unsigned char* blue) const
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		(*blue)  = *(itr++);
		(*green) = *(itr++);
		(*red)   = *(itr++);
	}
}

void bitmap::export_ycbcr(double* y, double* cb, double* cr)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++y, ++cb, ++cr)
	{
		double blue  = (1.0 * (*(itr++)));
		double green = (1.0 * (*(itr++)));
		double red   = (1.0 * (*(itr++)));

		( *y) = clamp<double>( 16.0 + (1.0/256.0) * (  65.738 * red + 129.057 * green +  25.064 * blue),1.0,254);
		(*cb) = clamp<double>(128.0 + (1.0/256.0) * (- 37.945 * red -  74.494 * green + 112.439 * blue),1.0,254);
		(*cr) = clamp<double>(128.0 + (1.0/256.0) * ( 112.439 * red -  94.154 * green -  18.285 * blue),1.0,254);
	}
}

void bitmap::export_rgb_normal(double* red, double* green, double* blue) const
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		(*blue)  = (1.0 * (*(itr++)));
		(*green) = (1.0 * (*(itr++)));
		(*red)   = (1.0 * (*(itr++)));
	}
}

void bitmap::export_rgb_normal(float* red, float* green, float* blue) const
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		(*blue)  = (1.0f * (*(itr++)));
		(*green) = (1.0f * (*(itr++)));
		(*red)   = (1.0f * (*(itr++)));
	}
}

void bitmap::import_rgb(double* red, double* green, double* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(256.0 * (*blue ));
		*(itr++) = static_cast<unsigned char>(256.0 * (*green));
		*(itr++) = static_cast<unsigned char>(256.0 * (*red  ));
	}
}

void bitmap::import_rgb(float* red, float* green, float* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(256.0f * (*blue ));
		*(itr++) = static_cast<unsigned char>(256.0f * (*green));
		*(itr++) = static_cast<unsigned char>(256.0f * (*red  ));
	}
}

void bitmap::import_rgb(unsigned char* red, unsigned char* green, unsigned char* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = (*blue );
		*(itr++) = (*green);
		*(itr++) = (*red  );
	}
}

void bitmap::import_ycbcr(double* y, double* cb, double* cr)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++y, ++cb, ++cr)
	{
		double y_  =  (*y);
		double cb_ = (*cb);
		double cr_ = (*cr);

		*(itr++) = static_cast<unsigned char>(clamp((298.082 * y_ + 516.412 * cb_                 ) / 256.0 - 276.836,0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp((298.082 * y_ - 100.291 * cb_ - 208.120 * cr_ ) / 256.0 + 135.576,0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp((298.082 * y_                 + 408.583 * cr_ ) / 256.0 - 222.921,0.0,255.0));
	}
}

void bitmap::import_rgb_clamped(double* red, double* green, double* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0 * (*blue ),0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0 * (*green),0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0 * (*red  ),0.0,255.0));
	}
}

void bitmap::import_rgb_clamped(float* red, float* green, float* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0f * (*blue ),0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0f * (*green),0.0,255.0));
		*(itr++) = static_cast<unsigned char>(clamp<double>(256.0f * (*red  ),0.0,255.0));
	}
}

void bitmap::import_rgb_normal(double* red, double* green, double* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(*blue );
		*(itr++) = static_cast<unsigned char>(*green);
		*(itr++) = static_cast<unsigned char>(*red  );
	}
}

void bitmap::import_rgb_normal(float* red, float* green, float* blue)
{
	if (bgr_mode != channel_arrangement_)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); ++red, ++green, ++blue)
	{
		*(itr++) = static_cast<unsigned char>(*blue );
		*(itr++) = static_cast<unsigned char>(*green);
		*(itr++) = static_cast<unsigned char>(*red  );
	}
}

void bitmap::subsample(bitmap& dest)
{
	/*
	Half sub-sample of original image.
	*/
	unsigned int w = 0;
	unsigned int h = 0;

	bool odd_width = false;
	bool odd_height = false;

	if (0 == (width % 2))
		w = width / 2;
	else
	{
		w = 1 + (width / 2);
		odd_width = true;
	}

	if (0 == (height % 2))
		h = height / 2;
	else
	{
		h = 1 + (height / 2);
		odd_height = true;
	}

	unsigned int horizontal_upper = (odd_width)  ? (w - 1) : w;
	unsigned int vertical_upper   = (odd_height) ? (h - 1) : h;

	dest.setwidth_height(w,h);
	dest.clear();

	unsigned char* s_itr[3];
	const unsigned char*  itr1[3];
	const unsigned char*  itr2[3];

	s_itr[0] = dest.data.get() + 0;
	s_itr[1] = dest.data.get() + 1;
	s_itr[2] = dest.data.get() + 2;

	itr1[0] = data.get() + 0;
	itr1[1] = data.get() + 1;
	itr1[2] = data.get() + 2;

	itr2[0] = data.get() + rowIncrement + 0;
	itr2[1] = data.get() + rowIncrement + 1;
	itr2[2] = data.get() + rowIncrement + 2;

	unsigned int total = 0;

	for (unsigned int j = 0; j < vertical_upper; ++j)
	{
		for (unsigned int i = 0; i < horizontal_upper; ++i)
		{
			for (unsigned int k = 0; k < bytesPerPixel; s_itr[k] += bytesPerPixel, ++k)
			{
				total = 0;
				total += *(itr1[k]); itr1[k] += bytesPerPixel;
				total += *(itr1[k]); itr1[k] += bytesPerPixel;
				total += *(itr2[k]); itr2[k] += bytesPerPixel;
				total += *(itr2[k]); itr2[k] += bytesPerPixel;

				*(s_itr[k]) = static_cast<unsigned char>(total >> 2);
			}
		}

		if (odd_width)
		{
			for (unsigned int k = 0; k < bytesPerPixel; s_itr[k] += bytesPerPixel, ++k)
			{
				total = 0;
				total += *(itr1[k]); itr1[k] += bytesPerPixel;
				total += *(itr2[k]); itr2[k] += bytesPerPixel;

				*(s_itr[k]) = static_cast<unsigned char>(total >> 1);
			}
		}

		for (unsigned int k = 0; k < bytesPerPixel; itr1[k] += rowIncrement, ++k);

		if (j != (vertical_upper - 1))
		{
			for (unsigned int k = 0; k < bytesPerPixel; itr2[k] += rowIncrement, ++k);
		}
	}

	if (odd_height)
	{
		for (unsigned int i = 0; i < horizontal_upper; ++i)
		{
			for (unsigned int k = 0; k < bytesPerPixel; s_itr[k] += bytesPerPixel, ++k)
			{
				total = 0;
				total += *(itr1[k]); itr1[k] += bytesPerPixel;
				total += *(itr2[k]); itr2[k] += bytesPerPixel;

				*(s_itr[k]) = static_cast<unsigned char>(total >> 1);
			}
		}

		if (odd_width)
		{
			for (unsigned int k = 0; k < bytesPerPixel; ++k)
			{
				(*(s_itr[k])) = *(itr1[k]);
			}
		}
	}
}

void bitmap::upsample(bitmap& dest)
{
	/*
	2x up-sample of original image.
	*/

	dest.setwidth_height(2 * width ,2 * height);
	dest.clear();

	const unsigned char* s_itr[3];
	unsigned char*  itr1[3];
	unsigned char*  itr2[3];

	s_itr[0] = data.get() + 0;
	s_itr[1] = data.get() + 1;
	s_itr[2] = data.get() + 2;

	itr1[0] = dest.data.get() + 0;
	itr1[1] = dest.data.get() + 1;
	itr1[2] = dest.data.get() + 2;

	itr2[0] = dest.data.get() + dest.rowIncrement + 0;
	itr2[1] = dest.data.get() + dest.rowIncrement + 1;
	itr2[2] = dest.data.get() + dest.rowIncrement + 2;

	for (unsigned int j = 0; j < height; ++j)
	{
		for (unsigned int i = 0; i < width; ++i)
		{
			for (unsigned int k = 0; k < bytesPerPixel; s_itr[k] += bytesPerPixel, ++k)
			{
				*(itr1[k]) = *(s_itr[k]); itr1[k] += bytesPerPixel;
				*(itr1[k]) = *(s_itr[k]); itr1[k] += bytesPerPixel;

				*(itr2[k]) = *(s_itr[k]); itr2[k] += bytesPerPixel;
				*(itr2[k]) = *(s_itr[k]); itr2[k] += bytesPerPixel;
			}
		}

		for (unsigned int k = 0; k < bytesPerPixel; ++k)
		{
			itr1[k] += dest.rowIncrement;
			itr2[k] += dest.rowIncrement;
		}
	}
}

void bitmap::alpha_blend(const double& alpha, const bitmap& image)
{
	if (
		(image.width  != width ) ||
		(image.height != height)
		)
	{
		return;
	}

	if ((alpha < 0.0) || (alpha > 1.0))
	{
		return;
	}

	unsigned char* itr1     = data.get();
	unsigned char* itr1_end = data.get() + length;
	unsigned char* itr2     = image.data.get();

	double alpha_compliment = 1.0 - alpha;

	while (itr1 != itr1_end)
	{
		*(itr1) = static_cast<unsigned char>((alpha * (*itr2)) + (alpha_compliment * (*itr1)));
		++itr1;
		++itr2;
	}
}

double bitmap::psnr(const bitmap& image)
{
	if (
		(image.width  != width ) ||
		(image.height != height)
		)
	{
		return 0.0;
	}

	unsigned char* itr1 = data.get();
	unsigned char* itr2 = image.data.get();

	double mse = 0.0;

	while (itr1 != (data.get() + length))
	{
		double v = (static_cast<double>(*itr1) - static_cast<double>(*itr2));

		mse += v * v;
		++itr1;
		++itr2;
	}

	if (mse <= 0.0000001)
	{
		return 1000000.0;
	}
	else
	{
		mse /= (3.0 * width * height);
		return 20.0 * std::log10(255.0 / std::sqrt(mse));
	}
}

double bitmap::psnr(const unsigned int& x,
					const unsigned int& y,
					const bitmap& image)
{
	if ((x + image._width()) > width)   { return 0.0; }
	if ((y + image._height()) > height) { return 0.0; }

	double mse = 0.0;

	const unsigned int _height = image._height();
	const unsigned int _width  = image._width();

	for (unsigned int r = 0; r < _height; ++r)
	{
		unsigned char* itr1       = row(r + y) + x * bytesPerPixel;
		unsigned char* itr1_end   = itr1 + (_width * bytesPerPixel);
		const unsigned char* itr2 = image.row(r);

		while (itr1 != itr1_end)
		{
			double v = (static_cast<double>(*itr1) - static_cast<double>(*itr2));
			mse += v * v;
			++itr1;
			++itr2;
		}
	}

	if (mse <= 0.0000001)
	{
		return 1000000.0;
	}
	else
	{
		mse /= (3.0 * image._width() * image._height());
		return 20.0 * std::log10(255.0 / std::sqrt(mse));
	}
}

void bitmap::histogram(const color_plane color, double hist[256])
{
	std::fill(hist,hist + 256,0.0);

	for (unsigned char* itr = (data.get() + offset(color)); itr < (data.get() + length); itr += bytesPerPixel)
	{
		++hist[(*itr)];
	}
}

void bitmap::histogram_normalized(const color_plane color, double hist[256])
{
	histogram(color,hist);

	double* h_itr = hist;
	const double* h_end = hist + 256;
	const double pixel_count = static_cast<double>(width * height);

	while (h_end != h_itr)
	{
		*(h_itr++) /= pixel_count;
	}
}

unsigned int bitmap::offset(const color_plane color)
{
	switch (channel_arrangement_)
	{
	case rgb_mode : {
		switch (color)
		{
		case red_plane   : return 0;
		case green_plane : return 1;
		case blue_plane  : return 2;
		default          : return std::numeric_limits<unsigned int>::max();
		}
					}

	case bgr_mode : {
		switch (color)
		{
		case red_plane   : return 2;
		case green_plane : return 1;
		case blue_plane  : return 0;
		default          : return std::numeric_limits<unsigned int>::max();
		}
					}

	default       : return std::numeric_limits<unsigned int>::max();
	}
}

void bitmap::incremental()
{
	unsigned char current_color = 0;

	for (unsigned char* itr = data.get(); itr < (data.get() + length);)
	{
		(*itr++) = (current_color);
		(*itr++) = (current_color);
		(*itr++) = (current_color);

		++current_color;
	}
}

bool bitmap::big_endian()
{
	unsigned int v = 0x01;

	return (1 != reinterpret_cast<char*>(&v)[0]);
}

unsigned short bitmap::flip(const unsigned short& v)
{
	return ((v >> 8) | (v << 8));
}

unsigned int bitmap::flip(const unsigned int& v)
{
	return (((v & 0xFF000000) >> 0x18) |
		((v & 0x000000FF) << 0x18) |
		((v & 0x00FF0000) >> 0x08) |
		((v & 0x0000FF00) << 0x08));
}

template<typename T>
void bitmap::read_from_stream(std::ifstream& stream,T& t)
{
	stream.read(reinterpret_cast<char*>(&t),sizeof(T));
}

template<typename T>
void bitmap::write_to_stream(std::ofstream& stream,const T& t)
{
	stream.write(reinterpret_cast<const char*>(&t),sizeof(T));
}

void bitmap::read_bfh(std::ifstream& stream, bitmap_file_header& bfh)
{
	read_from_stream(stream,bfh.type);
	read_from_stream(stream,bfh.size);
	read_from_stream(stream,bfh.reserved1);
	read_from_stream(stream,bfh.reserved2);
	read_from_stream(stream,bfh.off_bits);

	if (big_endian())
	{
		bfh.type      = flip(bfh.type);
		bfh.size      = flip(bfh.size);
		bfh.reserved1 = flip(bfh.reserved1);
		bfh.reserved2 = flip(bfh.reserved2);
		bfh.off_bits  = flip(bfh.off_bits);
	}
}

void bitmap::write_bfh(std::ofstream& stream, const bitmap_file_header& bfh)
{
	if (big_endian())
	{
		write_to_stream(stream,flip(bfh.type     ));
		write_to_stream(stream,flip(bfh.size     ));
		write_to_stream(stream,flip(bfh.reserved1));
		write_to_stream(stream,flip(bfh.reserved2));
		write_to_stream(stream,flip(bfh.off_bits ));
	}
	else
	{
		write_to_stream(stream,bfh.type     );
		write_to_stream(stream,bfh.size     );
		write_to_stream(stream,bfh.reserved1);
		write_to_stream(stream,bfh.reserved2);
		write_to_stream(stream,bfh.off_bits );
	}
}

void bitmap::read_bih(std::ifstream& stream,bitmap_information_header& bih)
{
	read_from_stream(stream,bih.size  );
	read_from_stream(stream,bih._width );
	read_from_stream(stream,bih._height);
	read_from_stream(stream,bih.planes);
	read_from_stream(stream,bih.bit_count);
	read_from_stream(stream,bih.compression);
	read_from_stream(stream,bih.size_image);
	read_from_stream(stream,bih.x_pels_per_meter);
	read_from_stream(stream,bih.y_pels_per_meter);
	read_from_stream(stream,bih.clr_used);
	read_from_stream(stream,bih.clr_important);

	if (big_endian())
	{
		bih.size        = flip(bih.size     );
		bih._width       = flip(bih._width    );
		bih._height      = flip(bih._height   );
		bih.planes      = flip(bih.planes   );
		bih.bit_count   = flip(bih.bit_count);
		bih.compression = flip(bih.compression);
		bih.size_image  = flip(bih.size_image);
		bih.x_pels_per_meter = flip(bih.x_pels_per_meter);
		bih.y_pels_per_meter = flip(bih.y_pels_per_meter);
		bih.clr_used = flip(bih.clr_used);
		bih.clr_important = flip(bih.clr_important);
	}
}

void bitmap::write_bih(std::ofstream& stream, const bitmap_information_header& bih)
{
	if (big_endian())
	{
		write_to_stream(stream,flip(bih.size));
		write_to_stream(stream,flip(bih._width));
		write_to_stream(stream,flip(bih._height));
		write_to_stream(stream,flip(bih.planes));
		write_to_stream(stream,flip(bih.bit_count));
		write_to_stream(stream,flip(bih.compression));
		write_to_stream(stream,flip(bih.size_image));
		write_to_stream(stream,flip(bih.x_pels_per_meter));
		write_to_stream(stream,flip(bih.y_pels_per_meter));
		write_to_stream(stream,flip(bih.clr_used));
		write_to_stream(stream,flip(bih.clr_important));
	}
	else
	{
		write_to_stream(stream,bih.size);
		write_to_stream(stream,bih._width);
		write_to_stream(stream,bih._height);
		write_to_stream(stream,bih.planes);
		write_to_stream(stream,bih.bit_count);
		write_to_stream(stream,bih.compression);
		write_to_stream(stream,bih.size_image);
		write_to_stream(stream,bih.x_pels_per_meter);
		write_to_stream(stream,bih.y_pels_per_meter);
		write_to_stream(stream,bih.clr_used);
		write_to_stream(stream,bih.clr_important);
	}
}

void bitmap::create_bitmap()
{
	length        = width * height * bytesPerPixel;
	rowIncrement = width * bytesPerPixel;
	data = shared_ptr<unsigned char>(new unsigned char[length]);
}

void bitmap::load_bitmap()
{
	std::ifstream stream(fileName.c_str(),std::ios::binary);

	if (!stream)
	{
		std::cerr << "bitmap::load_bitmap() ERROR: bitmap - file " << fileName << " not found!" << std::endl;
		return;
	}

	bitmap_file_header bfh;
	bitmap_information_header bih;

	read_bfh(stream,bfh);
	read_bih(stream,bih);

	if (bfh.type != 19778)
	{
		stream.close();
		std::cerr << "bitmap::load_bitmap() ERROR: bitmap - Invalid type value " << bfh.type << " expected 19778." << std::endl;
		return;
	}

	if (bih.bit_count != 24)
	{
		stream.close();
		std::cerr << "bitmap::load_bitmap() ERROR: bitmap - Invalid bit depth " << bih.bit_count << " expected 24." << std::endl;

		return;
	}

	height = bih._height;
	width  = bih._width;

	bytesPerPixel = bih.bit_count >> 3;

	unsigned int padding = (4 - ((3 * width) % 4)) % 4;
	char padding_data[4] = {0,0,0,0};

	create_bitmap();

	for (unsigned int i = 0; i < height; ++i)
	{
		unsigned char* data_ptr = row(height - i - 1); // read in inverted row order

		stream.read(reinterpret_cast<char*>(data_ptr),sizeof(char) * bytesPerPixel * width);
		stream.read(padding_data,padding);
	}
}

void bitmap::reverse_channels()
{
	if (3 != bytesPerPixel)
		return;

	for (unsigned char* itr = data.get(); itr < (data.get() + length); itr += bytesPerPixel)
	{
		unsigned char tmp = *(itr + 0);

		*(itr + 0) = *(itr + 2);
		*(itr + 2) = tmp;
	}
}

template<typename T>
T bitmap::clamp(const T& v, const T& lower_range, const T& upper_range)
{
	if (v < lower_range)
		return lower_range;
	else if (v >  upper_range)
		return upper_range;
	else
		return v;
}
