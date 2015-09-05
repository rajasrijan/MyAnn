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
	size_t matrix_order;
	const shared_ptr<double> data;
	Matrix();
	Matrix(size_t _rows, size_t _cols);
	Matrix(size_t _rows, size_t _cols, shared_ptr<double> _data);
	~Matrix();
	
	static void Mul(Matrix &a, Matrix &b, Matrix &out);
	static void Mul(double c,Matrix &out);
	static void Add(Matrix &a, Matrix &b, Matrix &out);
	static void Sub(Matrix &a, Matrix &b, Matrix &out);
	static void TransposeMul(Matrix &aT, Matrix &b, Matrix &out);
	static void deepCopy(Matrix& input,Matrix& output);
	Matrix transpose();
	void print()
	{
		for (size_t i = 0; i < rows; i++)
		{
			cout << "{";
			for (size_t j = 0; j < cols; j++)
			{
				cout << data.get()[(i*cols) + j] << " ";
			}
			cout << "}\n";
		}
	}
};

