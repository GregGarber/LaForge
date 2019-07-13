/* (C) 2016 EsonJohn(zhangyibupt@163.com) */

#include "CIC.h"

#include <assert.h>
#include <memory.h>

CIC::CIC(int decimationFactor, int numberOfSections, int diferrencialDelay):
	R(decimationFactor),
	N(numberOfSections),
	M(diferrencialDelay)
{
	buffer_comb = NULL;
	buffer_integrator = NULL;
	offset_comb = NULL;

	assert(R > 0 && N > 0 && M > 0);

	buffer_integrator = new double[N];
	offset_comb = new int[N];
	buffer_comb = new double*[N];
	for (int i = 0; i < N; i++)
		buffer_comb[i] = new double[M];
}

CIC::~CIC()
{
	// release buffer
	delete buffer_integrator;
	delete offset_comb;
	for (int i = 0; i < N; i++)
		delete buffer_comb[i];
	delete buffer_comb;
}
void CIC::setAttenuation(double attenuation){
	atten=attenuation;
}
void CIC::dump(){
	printf("\n buffer_integrator\n");
	for (int i=0; i<N; i++){
		printf("%f ", buffer_integrator[i]);
	}
	printf("\n offset_comb\n");
	for (int i=0; i<N; i++){
		printf("%d ", offset_comb[i]);
	}
	printf("\n buffer_comb\n");
	for (int i=0; i<N; i++){
		printf(" buffer_comb N:%d\n",i);
		for (int j=0; j<M; j++){
			printf("%f ", buffer_comb[i][j]);
		}
	}
}

void CIC::reset()
{
	// clear buffer
	for (int i = 0; i < N; i++)
	{
		buffer_integrator[i] = 0;
		offset_comb[i] = 0;
		for (int j = 0; j < M; j++)
			buffer_comb[i][j] = 0;
	}
}

double CIC::filter(double *input, int length)
{
	if (length != R)
		return 0;

	static double tmp_out = 0;
	
	// Integrator part
	for (int i = 0; i < R; i++)
	{
		tmp_out = input[i];

		for (int j = 0; j < N; j++)
			tmp_out = buffer_integrator[j] = buffer_integrator[j] + tmp_out;
	}

	// Comb part
	for (int i = 0; i < N; i++)
	{
		offset_comb[i] = (offset_comb[i] + 1) % M;
		double tmp = buffer_comb[i][offset_comb[i]];
		buffer_comb[i][offset_comb[i]] = tmp_out;
		tmp_out = tmp_out - tmp;
	}

	return atten * tmp_out;
}
