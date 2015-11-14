#include "Matrix.h"


Matrix::Matrix() :rows(0), cols(0), depth(0), matrix_order(ROW_MAJOR),gpuTransferNeeded(true), data(0),gpuData(rows*cols*depth)
{
	fromNeeded=true;
}

Matrix::Matrix(size_t _rows, size_t _cols, size_t _depth, bool initilize) : rows(_rows), cols(_cols), depth(_depth), matrix_order(ROW_MAJOR),gpuTransferNeeded(true), data(new float[_rows*_cols*_depth]),gpuData(_rows*_cols*_depth)
{
	for (size_t i = 0; initilize && (i < rows*cols*depth); i++)
	{
		data.get()[i] = 0.0f;
	}
	fromNeeded=true;
}

Matrix::Matrix(size_t _rows, size_t _cols, shared_ptr<float> _data) : rows(_rows), cols(_cols), depth(1), matrix_order(ROW_MAJOR),gpuTransferNeeded(true), data(_data),gpuData(_rows*_cols*1)
{
	fromNeeded=true;
}

Matrix::Matrix(size_t _rows, size_t _cols, size_t _depth, shared_ptr<float> _data) : rows(_rows), cols(_cols), depth(_depth), matrix_order(ROW_MAJOR),gpuTransferNeeded(true), data(_data),gpuData(_rows*_cols*_depth)
{
	fromNeeded=true;
}


Matrix::~Matrix()
{

}


void Matrix::Mul(Matrix &a, Matrix &b, Matrix &out)
{
	if ((a.cols != b.rows) || (out.cols == 0) || (out.rows == 0) || (out.rows != a.rows) || (out.cols != b.cols))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	size_t common_len=a.cols;
	array_view<float,2> a_view = a.gpuData.view_as(Concurrency::extent<2>(a.rows,a.cols));
	array_view<float,2> b_view = b.gpuData.view_as(Concurrency::extent<2>(b.rows,b.cols));
	array_view<float,2> output_view = out.gpuData.view_as(Concurrency::extent<2>(out.rows,out.cols));
	out.discardGpu();
	parallel_for_each(output_view.extent,[common_len,a_view,b_view,output_view](index<2> idx)restrict(amp)
	{
		float val=0;
		for(size_t i=0;i<common_len;i++)
		{
			float a=a_view[index<2>(idx[0],i)];
			float b=b_view[index<2>(i,idx[1])];
			val+=a*b;
		}
		output_view[idx]=val;
	});
	out.fromNeeded=true;
	/*for (size_t i = 0; i < a.rows; i++)
	{
	for (size_t j = 0; j < b.cols; j++)
	{
	float val = 0;
	for (size_t k = 0; k < a.cols; k++)
	{
	val += a.data.get()[(i*a.cols) + k] * b.data.get()[(k*b.cols) + j];
	}
	out.data.get()[(i*b.cols) + j] = val;
	}
	}*/
}

void Matrix::Mul(float* a, size_t a_rows, size_t a_cols, size_t a_stride, float* b, size_t b_rows, size_t b_cols, size_t b_stride, float* out, size_t out_rows, size_t out_cols, size_t out_stride)restrict(amp, cpu)
{
	for (size_t i = 0; i < a_rows; i++)
	{
		for (size_t j = 0; j < b_cols; j++)
		{
			float val = 0;
			for (size_t k = 0; k < a_cols; k++)
			{
				val += a[(i*(a_cols + a_stride)) + k] * b[(k*(b_cols + b_stride)) + j];
			}
			out[(i*(b_cols + out_stride)) + j] = val;
		}
	}
}

void Matrix::Mul(float c, Matrix &out)
{
	if ((out.cols == 0) || (out.rows == 0))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	out.toGpu();
	array<float,1> &myData = out.gpuData;
	parallel_for_each(out.gpuData.extent,[c,&myData](index<1> idx) restrict(amp)
	{
		myData[idx]=c*myData[idx];
	});
	out.fromNeeded=true;
	/*for (size_t i = 0; i < out.rows*out.cols; i++)
	{
	out.data.get()[i] *= c;
	}*/
}

void Matrix::Add(Matrix &a, Matrix &b, Matrix &out)
{
	if ((a.cols != b.cols) || (out.cols == 0) || (out.rows == 0) || (out.rows != a.rows) || (out.cols != a.cols))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	for (size_t i = 0; i < a.rows*a.cols; i++)
	{
		out.data.get()[i] = a.data.get()[i] + b.data.get()[i];
	}
}

void Matrix::Sub(Matrix &a, Matrix &b, Matrix &out)
{
	if ((a.cols != b.cols) || (out.cols == 0) || (out.rows == 0) || (out.rows != a.rows) || (out.cols != a.cols))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	for (size_t i = 0; i < a.rows*a.cols; i++)
	{
		out.data.get()[i] = a.data.get()[i] - b.data.get()[i];
	}
}

float* Matrix::getData()
{
	fromGpu();
	gpuTransferNeeded=true;
	return data.get();
}

int Matrix::size()
{
	return rows*cols*depth;
}

void Matrix::TransposeMul(Matrix &aT, Matrix &b, Matrix &out)
{
	if ((aT.rows != b.rows) || (out.cols == 0) || (out.rows == 0) || (out.rows != aT.cols) || (out.cols != b.cols))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	for (size_t i = 0; i < aT.cols; i++)
	{
		for (size_t j = 0; j < b.cols; j++)
		{
			float val = 0;
			for (size_t k = 0; k < aT.rows; k++)
			{
				val += aT.data.get()[i + (k*aT.cols)] * b.data.get()[(k*b.cols) + j];
			}
			out.data.get()[(i*b.cols) + j] = val;
		}
	}
}

void Matrix::deepCopy(Matrix& input, Matrix& output)
{
	if ((output.cols == 0) || (output.rows == 0) || (output.rows != input.rows) || (output.cols != input.cols))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	for (size_t i = 0; i < input.rows*input.cols; i++)
	{
		output.data.get()[i] = input.data.get()[i];
	}
}

void Matrix::toGpu()
{
	if(gpuTransferNeeded)
	{
		Concurrency::copy(data.get(),data.get()+(rows*cols*depth),gpuData);
		gpuTransferNeeded=false;
	}
}
void Matrix::discardGpu()
{
	gpuTransferNeeded=false;
}

void Matrix::fromGpu()
{
	if (fromNeeded)
	{
		Concurrency::copy(gpuData,data.get());
		fromNeeded=false;
	}
}

