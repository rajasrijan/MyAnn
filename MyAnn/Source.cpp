#include <iostream>
#include <memory>
#include <ctime>
#include <amp.h>

#include "Matrix.h"
#include "RowVector.h"

using namespace std;
using namespace Concurrency;

#define getIndex(x,xs,y,ys) ((x%xs) + (y*ys))

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

void trainLayerConstructiveDivergence(RowVector &input, Matrix &w, int iter)
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

void dummy(float* test) restrict(amp)
{
	test[0] = 1;
}

void convolve(Matrix& input, Matrix& w, Matrix& output, size_t window, size_t step)
{
	array_view<const float, 3> input_view(input.rows, input.cols, input.depth, input.data.get());
	array_view<const float, 2> weight_view(w.rows, w.cols, w.data.get());
	array_view<float, 2> output_view(output.rows, output.cols, output.data.get());
	output_view.discard_data();

	try
	{
		parallel_for_each(
			output_view.extent,
			[=](index<2> idx)restrict(amp)
		{
			dummy(&output_view[idx]);
		}
		);
	}
	catch (exception e)
	{
		cout << e.what();
		exit(1);
	}
}

int main()
{
	Matrix a(4, 4);
	Matrix b(4, 4);
	Matrix out(4, 4);
	for (size_t i = 0; i < 4*4; i++)
	{
		a.data.get()[i] = i;
		b.data.get()[i] = i + 1;
	}
	a.print();
	cout << "\n";
	b.print();
	cout << "\n";

	Matrix::Mul(a.data.get()+1, 2, 2, 2, b.data.get(), 2, 2, 2,out.data.get(),2,2,2);
	out.print();
	cout << "\n";
	return 0;
}
