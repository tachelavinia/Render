/*
*						Micro Renderer  - Algoritmi Paraleli si Distribuiti
*								Implementare : Lavinia Tache - 332CA
*/

#include "homework1.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Un renderer este un software care converteste elemente matematice2 sau 3 dimensionale(linii, cuburi, sfere
in pixelii unei imagini care le reprezinta.*/
int num_threads;
int resolution;

/* Initializarea presupune alocarea matricei de pixeli sub forma careia va fi stocata imaginea in memorie.
De asemenea, conform cerintei imaginea va fi setata la 255 ca valoarea pentru pixeli pentru ca fundalul sa fie alb.*/
void initialize(image *img) {
	int rowIndex, columnIndex;
	
	img->row = resolution;
	img->column = resolution;
	img->maxValue = 255;
	sprintf(img->format, "%s", "P5");

	img->data = (unsigned char **)malloc(resolution * sizeof(unsigned char*));
	for(rowIndex = 0; rowIndex < resolution; rowIndex++){
	    img->data[rowIndex] = (unsigned char*)malloc(resolution * sizeof(unsigned char));
	}

	for(rowIndex = 0; rowIndex < resolution; rowIndex++){
		for(columnIndex = 0; columnIndex < resolution; columnIndex++){
			img->data[rowIndex][columnIndex] = 255;
		}
	}
}

/* Pentru a completa linia neagra pe fundalul alb deja construit, am calculat distranta in plan de la fiecare
pixel pana la drapta reprezentata de ecuatia data. Daca aceasta distanta este mai mica decat 3 pixelul va fi
negru, altel ramane alb. Se construieste o linie cu grosimea de 3m.*/

void render(image *im) {
	int rowIndex, columnIndex, sum = 0, distance;
	omp_set_num_threads(num_threads);
	#pragma omp parallel for schedule(dynamic) private(sum, rowIndex, columnIndex, distance)

	for(rowIndex = 0; rowIndex < resolution; rowIndex++){
		for(columnIndex = 0; columnIndex < resolution; columnIndex++){
			sum = (-1 * (columnIndex * 100 / resolution)) + (2 * ((resolution - 1 - rowIndex) * 100 / resolution));
			distance = abs(sum) / sqrt(5);
			
			if(distance < 3.0){
				im->data[rowIndex][columnIndex] = 0;
			}
		}
	}
}

/*De asemenea, pentru scrierea imaginii construite se va completa matricea de pixeli */
void writeData(const char * fileName, image *img){
	int rowIndex, columnIndex;
	FILE *file = fopen(fileName, "wb");
	if(file == NULL){
		printf("Unable to open for writing! \n");
		return;
	}

	fprintf(file, "%s\n", img->format);
	fprintf(file, "%d %d\n", img->row, img->column);
	fprintf(file, "%d\n", img->maxValue);

	for(rowIndex = 0; rowIndex < resolution; rowIndex++){
		for(columnIndex = 0; columnIndex < img->column; columnIndex++){
	    	fwrite(&img->data[rowIndex][columnIndex], sizeof(unsigned char), 1 , file);
		}
	}
}

