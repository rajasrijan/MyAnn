#include "ColVector.h"


ColVector::ColVector()
{
}

ColVector::ColVector(size_t size) : Matrix(1, size)
{
}
ColVector::ColVector(size_t size, shared_ptr<float> data) : Matrix(1,size,data)
{
}

ColVector::~ColVector()
{
}
