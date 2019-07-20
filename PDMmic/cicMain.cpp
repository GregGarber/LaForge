#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CIC.h"
int main () {
	//double data[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	double data[]={16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
	double ig=0;
  	CIC *cic = new CIC(8, 2, 16); 
	cic->setAttenuation(1.0); 
	
	for (int i=0; i<1000000; i++){
		//ig = cic->filter(data);
		ig = cic->filter(data,8);
	}
		printf("\t%f\n",ig);

	return 0;
}

