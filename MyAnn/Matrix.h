#pragma once
#include <iostream>
#include <memory>

using namespace std;
enum
{
	COLUMN_MAJOR,
	ROW_MAJOR
};
class Matrix
{
public:
	const size_t rows;
	const size_t cols;
	const size_t depth;
	size_t stride;
	size_t matrix_order;
	shared_ptr<float> data;
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
	Matrix transpose();
	void print()
	{
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

