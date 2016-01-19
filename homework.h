#ifndef HOMEWORK_H
#define HOMEWORK_H

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

/* structura pentru definirea unui pixel de tip RGB*/
typedef struct{
	unsigned char red, green, blue;
}PixelRGB;

/* Structura pentru definirea unui imagini : contine elementele specifice formatului si cele doua matrice detaliate in .c*/
typedef struct{
	int  row, column;
	int maxValue;
	char format[128];
	unsigned char **data;
	PixelRGB **dataRGB;
} image;

	void readInput(const char * fileName, image *img);
	void writeData(const char * fileName, image *img);
	void resize(image *in, image * out);

#endif
