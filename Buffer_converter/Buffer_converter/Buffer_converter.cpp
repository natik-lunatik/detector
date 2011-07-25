// Buffer_converter.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

// Buffer_decoder.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>  
#include <math.h>  
#include <stdio.h>  
#include <windows.h>

BOOL WINAPI DllMain()
{
	return TRUE;
}

unsigned long MAKE_WORD32(unsigned long* x, unsigned long i) 
{
	return (unsigned long)x[i] | ((unsigned long)x[i + 1]) << 16;
}

extern "C" _declspec (dllexport) int Convert_buf ( unsigned long* buffer, unsigned long bufferLen,  const char* file_name);

int Convert_buf ( unsigned long* buffer, unsigned long bufferLen,  const char* file_name)
{
	int i;

	unsigned long upperTimeWords[4];
	unsigned long nEventRecords;
	unsigned long nSpecialRecords;
	unsigned long nTotalRecords;
	unsigned long j;

	char file_name_i [256];
	FILE* f[4] = {NULL, NULL, NULL, NULL};

	for (int k=0; k<4; k++)
	{ 
		sprintf (file_name_i, "%s_%d.txt", file_name, k);
		f[k] = fopen(file_name_i, "wt");
		if (f[k] == NULL)
			return -1;
	}

	/* Assume that the variable buffer is already filled in up to bufferLen
	 * with the header and event data.
	 */

	/* Load the current upper time words from the buffer header. The
	 * upper time words for channel 0 begin at offset 72 in the header.
	 * And each channel has 12 words of data with it, so we need to increment
	 * by that much for each channel.
	 */
	for (i = 0; i < 4; i++) {
		upperTimeWords[i] = MAKE_WORD32(buffer, 72 + (i * 12));
	}

	/* Get the number of non-special records from the buffer header. */
	nEventRecords = MAKE_WORD32(buffer, 66);

	/* Get the number of special records from the buffer header. */
	nSpecialRecords = MAKE_WORD32(buffer, 116);

	nTotalRecords = nEventRecords + nSpecialRecords;

	//fprintf(f, "nTotalRecords %d\n", nTotalRecords);
	for (j = 0; j < nTotalRecords; j++) {
		unsigned long record[3];

		/* Copy each event into its own record for further processing. The
		 * buffer header is 256 words and each record is 3 words.
		 */
		memcpy(&record[0], &buffer[256 + (j * 3)], 3 * sizeof(unsigned long));

		if ((record[0] & 0x8000) > 0) {
		   /* This is a special record. */

		   if (record[0] == 0x8000) {
			  /* We have hit the end of the buffer. */
			  break;

		   } else {
			  /* This is a rollover special record. We need to update the
			   * upper word for the appropriate channel. The channel that
			   * this rollover record describes lives in the lower 4 bits of
			   * the first word of the record. The remaining two words are the
			   * the new upper time words for that channel.
			   */
			  int chan = record[0] & 0x000F;
			  upperTimeWords[chan] = MAKE_WORD32(record, 1);
		   }

		} else {
		   /* Normal event record. The channel is stored in bits 13 and 14 of
			* the first word and the energy bin is stored in the lower 13 bits.
			*/
		   unsigned long chan = (record[0] & 0x6000) >> 13;
		   unsigned long bin = record[0] & 0x1FFF;

		   /* Use double and ldexp() to create a 64-bit value. Not all platforms
			* support 64-bit integral types.
			*/
		   double timestamp = ldexp((double)upperTimeWords[chan], 32) +
							  (double)record[1] +
							  ldexp((double)record[2], 16);

		   fprintf(f[chan], "%0.1f\t%hu\n", timestamp, bin);
		   //printf("Timestamp %0.1f, Bin %hu\n", timestamp,
				 //  bin);
		}
	}
	for (int k=0; k<4;k++)
	{
		fclose(f[k]);
	}
	return 0;
};