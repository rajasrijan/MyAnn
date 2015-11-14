#define NOMINMAX
#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <memory>
#include <ctime>
#include <amp.h>
#include <algorithm>

#include "Matrix.h"
#include "RowVector.h"
#include "bitmap.h"
#include "my_view.h"

using namespace std;
using namespace Concurrency;

#define getIndex(x,xs,y,ys) ((x%xs) + (y*ys))

Matrix createLayer(int input, int output)
{
	return Matrix(output, input);
}

Matrix CreateConvLayer(int window,int channels,int features)
{
	Matrix ret(features,window*window*channels);
	size_t seed = (int)time(0);
	for (size_t i = 0; i < ret.rows*ret.cols*ret.depth; i++)
	{
		ret.getData()[i]=sinf((float)(i*i+seed)/180.0f);
	}
	return ret;
}

void initLayerRandom(Matrix &w)
{
	for (size_t i = 0; i < w.rows*w.cols; i++)
	{
		w.getData()[i] = sinf((float)(unsigned)time(0) * (float)i);
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

void MatrixMul(im2col &view,array_view<float,2> weight,float* output,int cols,int rows,int index) restrict(cpu,amp)
{
	for (int i = 0; i < cols; i++)
	{
		float val=0;
		for (int j = 0; j < rows; j++)
		{
			val += view.at(j,index)*weight[i][j];
		}
		output[i]=val;
	}
}

void convolve(Matrix& input, Matrix& w, Matrix& output, const int window, const size_t step,bool test)
{
	if (((window*window*input.depth) != w.cols) || ((output.depth) != w.rows) || (w.depth != 1))
	{
		cout << "Sanity check failed.Insane input.\n";
		throw("Sanity check failed.Insane input.");
	}
	if(test)
	{
		array_view<float,1> input_view(input.size(),input.getData());
		array_view<float,1> output_view(output.size(),output.getData());
		array_view<float,2> weight(w.rows,w.cols,w.getData());

		int output_channel = output.depth;
		int weight_window = w.cols;
		int rows=input.rows,cols=input.cols,depth=input.depth;

		parallel_for_each(
			Concurrency::extent<1>(output.rows*output.cols),
			[=](index<1> idx)__GPU
		{
			im2col view(input_view,cols,rows,depth,window,window);
			MatrixMul(view,weight,&output_view[(idx*output_channel)],output_channel,weight_window,idx[0]);
		}
		);
		output_view.synchronize();
	}
	else
	{
		array_view<float, 3> input_view(input.rows, input.cols, input.depth, input.getData());
		array_view<float, 2> weight_view(w.rows, w.cols, w.getData());
		array_view<float, 3> output_view(output.rows, output.cols, output.depth, output.getData());
		//output_view.discard_data();

		const int x_lim = output.rows;
		const int y_lim = output.cols;
		const int depth = w.rows;

		try
		{
			parallel_for_each(
				Concurrency::extent<2>(x_lim, y_lim),
				[=](index<2> idx)restrict(amp)
			{
				int half_range = (window / 2);
				for (int d = 0; d < depth; d++)
				{
					float val = 0;
					//	magic
					array_view<const float, 3> weight_subview(window, window,3, &weight_view[index<2>(d, 0)]);
					//	Matrix multiplication shortcut a.k.a magic
					for (int x = 0; x < window; x++)
					{
						for (int y = 0; y < window; y++)
						{
							for (int c = 0; c < 3; c++)
							{
								index<3> i = index<3>(idx[0] + x - half_range, idx[1] + y - half_range, c);
								if ((i[0] >= 0) && (i[1] >= 0) && (i[0] < x_lim) && (i[1] < y_lim))
								{
									val += weight_subview[index<3>(x, y,c)] * input_view[i];
								}
							}
						}
					}
					output_view[index<3>(idx[0], idx[1], d)] = val;
				}
			}
			);
			output_view.synchronize();

		}
		catch (exception e)
		{
			cout << e.what();
			exit(1);
		}
	}
}

void convLayerConstructiveDivergence(Matrix& input, Matrix& w, Matrix& output, const int window, const size_t step=1,int iter=1)
{
	if (((window*window*input.depth) != w.cols) || ((output.depth) != w.rows) || (w.depth != 1))
	{
		cout << "Sanity check failed.Insane input.\n";
		throw("Sanity check failed.Insane input.");
	}
	array_view<float,1> input_view(input.size(),input.getData());
	array_view<float,1> output_view(output.size(),output.getData());
	array_view<float,2> weight(w.rows,w.cols,w.getData());

	int output_channel = output.depth;
	int weight_window = w.cols;
	int rows=input.rows,cols=input.cols,depth=input.depth;

	parallel_for_each(
		Concurrency::extent<1>(output.rows*output.cols),
		[=](index<1> idx)__GPU
	{
		//	1.Take a training sample v, compute the probabilities of the hidden units and sample a hidden activation vector h from this probability distribution.
		im2col v(input_view,cols,rows,depth,window,window);
		MatrixMul(v,weight,&output_view[(idx*output_channel)],output_channel,weight_window,idx[0]);
	}
	);
	output_view.synchronize();
}

int main()
{
	/*Matrix m(4,4,3);
	Matrix w(3,27);
	Matrix o(4,4,3,true);
	float *data = m.getData();
	for(int i=0;i<m.size();i++)data[i]=(float)i+1;
	for(int i=0;i<w.size();i++)w.getData()[i]=(float)1;
	m.print();
	cout<<"\n\n";
	w.print();
	cout<<"\n\n";
	convolve(m,w,o,3,1);
	o.print();
	cout<<"\n\n";*/

	//im2col<float,float*> v(4,4,3,1,1);
	//cout<<v.at(data,0,0)<<'\n';


	bitmap img("test.bmp");
	bitmap img2;
	Matrix a = img.getMatrix();
	Matrix w=CreateConvLayer(3,3,3);
	Matrix out(a.rows,a.cols,a.depth);

	convolve(a,w,out,3,1,true);

	img2.setMatrix(out);
	img2.save_image("data1.bmp");

	return 0;
}
