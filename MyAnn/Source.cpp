#include <iostream>
#include <memory>
#include <ctime>
#include "Matrix.h"
#include "RowVector.h"
using namespace std;

#define getIndex(x,xs,y,ys) ((x%xs) + (y*ys))

typedef double const * const INP;
typedef double * const OUT;

Matrix createLayer(int input, int output)
{
	return Matrix(output, input);
}

void initLayerRandom(Matrix &w)
{
	for (size_t i = 0; i < w.rows*w.cols; i++)
	{
		w.data.get()[i] = sin((unsigned)time(0) * i);
	}
}

void getLayerForward(RowVector &in, Matrix &w, RowVector &out)
{
	Matrix::Mul(w, in, out);
}

void getLayerBackward(RowVector &in, Matrix &w, RowVector &out)
{
	Matrix::Mul(in.getColVector(), w, out.getColVector());
}

void trainLayerConstructiveDivergence(RowVector &input, Matrix &w,int iter)
{
	float alpha = 0.01;
	RowVector v(input.rows);
	RowVector h(w.rows);
	Matrix positiveGradient(w.rows, w.cols);
	Matrix negativeGradient(w.rows, w.cols);
	Matrix weightUpdate(w.rows, w.cols);
	for (size_t i = 0; i < iter; i++)
	{
		w.print();
		cout << "\n";

		Matrix::deepCopy(input, v);
		v.print();
		cout << "\n";
		//	1.Take a training sample v, compute the probabilities of the hidden units and sample a hidden activation vector h from this probability distribution.
		getLayerForward(v, w, h);
		h.print();
		cout << "\n";

		//	2.Compute the outer product of v and h and call this the positive gradient.
		Matrix::Mul(h, v.getColVector(), positiveGradient);
		/*positiveGradient.print();
		cout << "\n";*/

		//	3.From h, sample a reconstruction v' of the visible units, then resample the hidden activations h' from this. (Gibbs sampling step)
		getLayerBackward(h, w, v);
		/*v.print();
		cout << "\n";*/
		getLayerForward(v, w, h);
		/*h.print();
		cout << "\n";*/

		//	4.Compute the outer product of v' and h' and call this the negative gradient.
		Matrix::Mul(h, v.getColVector(), negativeGradient);
		/*positiveGradient.print();
		cout << "\n";*/

		//	5.Let the weight update to w_{ i, j } be the positive gradient minus the negative gradient, times some learning rate : \Delta w_{ i, j } = \epsilon(vh^\mathsf{ T } -v'h'^\mathsf{ T }).
		Matrix::Sub(positiveGradient, negativeGradient, weightUpdate);
		Matrix::Mul(alpha, weightUpdate);
		weightUpdate.print();
		cout << "\n";
		Matrix::Add(w, weightUpdate, w);
	}
}

void convolve2D(RowVector& input,Matrix)
{

}

int main()
{
	RowVector input(1920*1080);	//Image as row vector
	return 0;
}
