/* (C) 2016 EsonJohn(zhangyibupt@163.com) */

#include "CIC.h"

#include <assert.h>
#include <memory.h>

CIC::CIC(int decimationFactor, int numberOfSections, int diferrencialDelay):
	R(decimationFactor),
	N(numberOfSections), // Getting rid of N, only N=2 seems useful
	M(diferrencialDelay)
{
	buffer_comb = NULL;
	buffer_integrator = NULL;
	integrator1=0.0;
	integrator2=0.0;
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
	integrator1=0.0;
	integrator2=0.0;
	comb_offset=0;
	tmp_out = 0.0;
	tmp=0.0;
}

// Hopefully hotrodded version. 1 Million calls takes about 0.180 seconds,
// not exactly a hot rod, but an improvement at least.
// Note N=2 and cannot change. N=1 or N=3 might be useful in some situations,
// but I dont think I've seen it here. If needed, just add new functions.
double CIC::filter(double *input) {
	tmp_out = 0.0;
	tmp=0.0;
	// Integrator part
	//RNM: 8 2 16, and  input 1,2,3,...16

	//revised like my asm
	// ** untried, but may be able to do like: integrator2=sum(v(1:8).*m, where v=input and m=[R R-1 R-2 ... 1] on GPU
	for (int i = 0; i < R; i++)  {
		integrator1 += input[i];
		integrator2 += integrator1;
	}
	//printf("integrator1=%f integrator2=%f\n",integrator1,integrator2);
	tmp_out = integrator2;

	// N 1
	comb_offset++;
	comb_offset = comb_offset % M;
	tmp = buffer_comb[0][comb_offset];
	buffer_comb[0][comb_offset] = tmp_out;
	tmp_out -= tmp;

	// N 2
	tmp = buffer_comb[1][comb_offset];
	buffer_comb[1][comb_offset] = tmp_out;
	tmp_out -= tmp;
	//printf("comb[0]=%f comb[1]=%f\n",buffer_comb[0][comb_offset],buffer_comb[1][comb_offset]);//does diverge
	return atten * tmp_out;
}

// The original version, 1 million calls takes about 0.330 seconds
double CIC::filter(double *input, int length)
{
	if (length != R)
		return 0;

	tmp_out = 0.0;
	tmp=0.0;
	
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
		tmp = buffer_comb[i][offset_comb[i]];
		buffer_comb[i][offset_comb[i]] = tmp_out;
		tmp_out = tmp_out - tmp;
	}

	return atten * tmp_out;
}
