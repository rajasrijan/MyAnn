#pragma once
#define NOMINMAX
#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <memory>
#include <amp.h>

using namespace std;
using namespace Concurrency;

enum
{
	COLUMN_MAJOR,
	ROW_MAJOR
};

class Matrix
{
private:
	shared_ptr<float> data;
public:
	friend class RowVector;
	size_t rows;
	size_t cols;
	size_t depth;
	size_t stride;
	size_t matrix_order;
	bool gpuTransferNeeded;
	bool fromNeeded;
	array<float,1> gpuData;

	Matrix();
	Matrix(size_t _rows, size_t _cols, size_t _depth = 1, bool initilize = false);
	Matrix(size_t _rows, size_t _cols, shared_ptr<float> _data);
	Matrix(size_t _rows, size_t _cols, size_t _depth, shared_ptr<float> _data);
	~Matrix();

	static void Mul(Matrix &a, Matrix &b, Matrix &out);
	static void Mul(float* a, size_t a_rows, size_t a_cols, size_t a_stride, float* b, size_t b_rows, size_t b_cols, size_t b_stride, float* out, size_t out_rows, size_t out_cols, size_t out_stride)restrict(amp, cpu);
	static void Mul(float c, Matrix &out);
	static void Add(Matrix &a, Matrix &b, Matrix &out);
	static void Sub(Matrix &a, Matrix &b, Matrix &out);
	static void TransposeMul(Matrix &aT, Matrix &b, Matrix &out);
	static void deepCopy(Matrix& input, Matrix& output);
	float* getData();
	Matrix transpose();
	int size();
	void toGpu();
	void discardGpu();
	void fromGpu();
	void print()
	{
		toGpu();
		fromGpu();
		for (size_t i = 0; i < rows; i++)
		{
			cout << "[ ";
			for (size_t j = 0; j < cols; j++)
			{
				if (depth > 1)
					cout << "(";
				for (size_t k = 0; k < depth; k++)
				{
					cout << data.get()[(((i*cols) + j)*depth) + k] << ((k == (depth - 1)) ? "" : ",");
				}
				if (depth > 1)
					cout << ")";
				cout << " ";
			}
			cout << "]\n";
		}
	}
};

class im2col
{
	int width,height,depth,_wx,_wy;
	array_view<float,1> data;
public:
	im2col(array_view<float,1> _data,int x,int y,int channel,int window_x,int window_y)__GPU
		: data(_data),width(x),height(y),depth(channel),_wx(window_x),_wy(window_y)
	{

	}
	~im2col()__GPU
	{

	}
	float at(int x,int y)__GPU
	{
		int a,b,channel;

		channel=x%depth;
		x/=depth;

		a=(x%_wx) - (_wx/2) + (y%width);

		b=(x/_wx) - (_wy/2) + (y/width);

		if((a<0)||(b<0)||(a>=width)||(b>=height))
			return 0.0;
		return data[channel + (depth*(a + (b*width)))];
	}
};
