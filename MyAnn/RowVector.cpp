#include "RowVector.h"


RowVector::RowVector()
{
}

RowVector::RowVector(int size) : Matrix(size,1)
{
}


RowVector::~RowVector()
{
}

ColVector RowVector::getColVector()
{
	return ColVector(rows, data);
}