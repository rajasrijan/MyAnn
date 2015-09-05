#include "ColVector.h"


ColVector::ColVector()
{
}

ColVector::ColVector(size_t size) : Matrix(1, size)
{
}
ColVector::ColVector(size_t size, shared_ptr<double> data) : Matrix(1,size,data)
{
}

ColVector::~ColVector()
{
}
