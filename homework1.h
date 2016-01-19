#ifndef HOMEWORK1_H
#define HOMEWORK1_H

#include <omp.h>
#include <stdlib.h>

/* Structura pentru definirea tipului imaginii */#include <stdio.h>
typedef struct{
	int  row, column;
	int maxValue;
	char format[128];
	unsigned char **data;
} image;

	void initialize(image *img);
	void render(image *im);
	void writeData(const char * fileName, image *img);

#endif
