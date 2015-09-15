#pragma once
#include "Matrix.h"
class ColVector :
	public Matrix
{
public:
	ColVector();
	ColVector(size_t size);
	ColVector(size_t size, shared_ptr<float> data);
	~ColVector();
};

