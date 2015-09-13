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
		w.data.get()[i] = sinf((float)(unsigned)time(0) * (float)i);
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

void trainLayerConstructiveDivergence(RowVector &input, Matrix &w, size_t iter)
{
	float alpha = 0.01f;
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

void convolve(Matrix& input, Matrix& w, Matrix& output, const int window, const size_t step)
{
	if ((window*window*input.depth)!=w.cols)
	{
		cout<<"weight size and window size dont match.";
		throw("weight size and window size dont match.");
	}
	array_view<const float, 3> input_view(input.rows, input.cols,input.depth, input.data.get());
	array_view<const float, 2> weight_view(w.rows, w.cols, w.data.get());
	array_view<float, 3> output_view(output.rows, output.cols,output.depth, output.data.get());
	output_view.discard_data();

	const int x_lim = output.rows;
	const int y_lim = output.cols;
	const int depth = w.cols;

	try
	{
		parallel_for_each(
			Concurrency::extent<2>(x_lim,y_lim),
			[=](index<2> idx)restrict(amp)
		{
			float val = 0;
			int half_range = (window / 2);
			for (int d = 0; d < depth; d++)
			{
				//	magic
				array_view<const float, 2> weight_subview(window, window, &weight_view[index<2>(0, 0)]);
				//	Matrix multiplication shortcut a.k.a magic
				for (int x = 0; x < window; x++)
				{
					for (int y = 0; y < window; y++)
					{
						index<3> i = index<3>(idx[0] + x - half_range,idx[1] + y - half_range,d);
						if ((i[0] >= 0) && (i[1] >= 0) && (i[0] < x_lim) && (i[1] < y_lim))
							val += weight_subview[index<2>(x, y)] * input_view[i];
					}
				}
				output_view[index<3>(idx[0],idx[1],d)] = val;
			}
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
	Matrix w(1, 9*3,1,true);
	Matrix out(4, 4);
	for (size_t i = 0; i < a.rows*a.cols*a.depth; i++)
	{
		a.data.get()[i] = (float)i;
	}
	for (size_t i = 0; i < 9 * 1; i++)
	{
		w.data.get()[i] = (float)i + 1.0f;
	}
	a.print();
	cout << "\n";
	w.print();
	cout << "\n";

	convolve(a, w, out, 3, 1);
	out.print();
	cout << "\n";
	return 0;
}
