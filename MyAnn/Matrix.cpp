#include "Matrix.h"


Matrix::Matrix() :rows(0), cols(0), matrix_order(ROW_MAJOR), data(0)
{
}

Matrix::Matrix(size_t _rows, size_t _cols) : rows(_rows), cols(_cols), matrix_order(ROW_MAJOR), data(new double[_rows*_cols])
{
	
}

Matrix::Matrix(size_t _rows, size_t _cols, shared_ptr<double> _data) : rows(_rows), cols(_cols), matrix_order(ROW_MAJOR), data(_data)
{
	
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
	for (size_t i = 0; i < a.rows; i++)
	{
		for (size_t j = 0; j < b.cols; j++)
		{
			double val = 0;
			for (size_t k = 0; k < a.cols; k++)
			{
				val += a.data.get()[(i*a.cols) + k] * b.data.get()[(k*b.cols) + j];
			}
			out.data.get()[(i*b.cols) + j] = val;
		}
	}
}

void Matrix::Mul(double c, Matrix &out)
{
	if ((out.cols == 0) || (out.rows == 0))
	{
		cout << "Matrix multiplication error.\n";
		return;
	}
	for (size_t i = 0; i < out.rows*out.cols; i++)
	{
		out.data.get()[i] *= c;
	}
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
			double val = 0;
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