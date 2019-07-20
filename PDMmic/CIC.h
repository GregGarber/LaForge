/* (C) 2016 EsonJohn(zhangyibupt@163.com) */

#ifndef CIC_H
#define CIC_H

#include <stdio.h>
#include <cmath>

class CIC
{
public:
	// R:Decimation factor;
	// N:Number of sections;
	// M:Differential delay;
	CIC(int decimationFactor, int numberOfSections, int diferrencialDelay);

	// destructor
	~CIC();

	void setAttenuation(double attenuation=1.0);
	void dump(); //show contents of all variables

	// the actual filter function
	// the parameter input shuld be R-dimensional vector(R continuous samples) and the parameter length should be R
	// the output is double, corresponding to the downsampled output
	double filter(double *input); // N=2, no R check
	double filter(double *input, int length);

	// reset the buffer
	void reset();

private:
	int			R, N, M;
	double 			atten = 1.0;
	double		*buffer_integrator;	// buffer of the integrator part
	double		integrator1;	// buffer of the integrator part
	double		integrator2;	// buffer of the integrator part
	double		**buffer_comb;		// buffer of the comb part
	double tmp_out = 0.0;
	double tmp=0.0;
	int			*offset_comb;
	int			comb_offset=0;

};

#endif
