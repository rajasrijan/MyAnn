#pragma once
#include <amp.h>

using namespace Concurrency;

template <typename _T, int _R>
class my_view:public array<_T,_R>
{
public:
	my_view(void);
	~my_view(void);
};

