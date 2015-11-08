#pragma once
#include "Matrix.h"
#include "ColVector.h"
class RowVector :
	public Matrix
{
public:
	RowVector();
	RowVector(int size);
	~RowVector();
	ColVector getColVector();
};

