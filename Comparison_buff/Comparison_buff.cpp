// Comparison_buff.cpp : Defines the entry point for the console application.
//

#include <math.h>
#include "stdafx.h"

// returns true if the line is successfully read
bool fileReadLine (FILE* f, double* time, int* bin) 
{
	return fscanf(f, "%lf%d", time, bin) == 2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	FILE* f0 = fopen("chan0.txt", "rt");
	FILE* f1 = fopen("chan1.txt", "rt");

	double time0 = -1, time1 = -1;
	int bin0 = -1, bin1 = -1;
	
	if (!fileReadLine(f0, &time0, &bin0) || !fileReadLine(f1, &time1, &bin1))
		return -1;
	while (true) {
		// compare timestamps
		if (fabs(time1 - time0) < 0.5) {
			// event timestamp match
			printf("event match %lf, %lf; enegries %d and %d\n", time0, time1, bin0, bin1);
		}
		// take the next instance from one of the files
		if (time1 > time0) {
			if (feof(f0))
				break;
			if (!fileReadLine(f0, &time0, &bin0))
				return -1;
		} else {
			if (feof(f1))
				break;
			if (!fileReadLine(f1, &time1, &bin1))
				return -1;
		}
	}

	scanf("*");
	return 0;
}