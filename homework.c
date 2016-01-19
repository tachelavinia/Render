/*
* 							Tema 1 - ALGORITMI PARALELI SI DISTRIBUITI
*						Micro Render si Resize folosind tehnici antialiansing
*							Realizata de : Lavinia Tache - 322CA
*/

#include "homework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int num_threads;
int resize_factor;

/* Functia de citire a inputului trateaza corner-cases pentru cazurile de eroare : formatul incorect pentru
input sau erori in deschidere pozei ce va urma sa fie editata. Imaginea este tinuta in memorie sub forma unei matrice
de pixeli, tranand in mod special doua cauzuri: 
	Primul caz: poza este alb-negru, iar pixelul este reprezentat in memorie ca un unsigned char initial la valoarea
corespunzatoare, iar poza o matrice de atfel de pixeli
	Al doilea caz: poza este color, iar pixelul este o structura pentru valorile RGB. Poza este reprezentata de o matrice
de astfel de structuri, in care fiecare culoare este initializata la valoarea corespunzatoare.*/

void readInput(const char * fileName, image *img) {
	FILE *file = fopen(fileName, "rb");
	char magicNum[128];
	int row, column, maxVal, rowIndex;

	fscanf(file, "%s\n%d\n %d\n%d\n", magicNum, &row, &column, &maxVal);
	img->row = row;
	img->column = column;
	img->maxValue = maxVal;
	sprintf(img->format, "%s", magicNum);

	if (strcmp(magicNum, "P5") != 0 && strcmp(magicNum, "P6") != 0)
    {
        fprintf(stderr, "Unable to read from file, because it is not a PNM file of type P6/P5\n");
        return;
    }

	if(file == NULL){
		fprintf(stderr, "Unable to open file %s\n", fileName);
	}

	if(strcmp(img->format, "P5") == 0){

		img->data = (unsigned char **)calloc(column , sizeof(unsigned char*));
	    for(rowIndex = 0; rowIndex < column; rowIndex++){
	    	img->data[rowIndex] = (unsigned char*)calloc(row , sizeof(unsigned char));
	    	fread(img->data[rowIndex], sizeof(unsigned char), row , file);
	    }
	}
	if(strcmp(img->format, "P6") == 0){

		img->dataRGB = (PixelRGB **)calloc(column , sizeof(PixelRGB*));
	    for(rowIndex = 0; rowIndex < column; rowIndex++){
	    	img->dataRGB[rowIndex] = (PixelRGB*)calloc(row , sizeof(PixelRGB));
	    	fread(img->dataRGB[rowIndex], sizeof(PixelRGB), row , file);
	    }
	}
	
    fclose(file); 
}

/* Pentru a scrie o poza in formatul corect se vor adauga initial magicNumber, dimensiunea matricei si maximul
valorii continute in pixel. MagicNumber reprezinta formatul pentru Netpbm format (un format grafic definit si
mentinut de Netpbm) care diferentiaza de asemenea grayscale si RGB.
La nivelul implementarii diferenta intre grayscale si RGB se face prin scriarea in matrice diferite - 
corespunzatoare celor tipuri de poze.*/

void writeData(const char * fileName, image *img) {
	int rowIndex, columnIndex;
	FILE *file = fopen(fileName, "wb");
	/* corner case : imposibilitatea deschiderii fisierului*/
	if(file == NULL){
		printf("Unable to open for writing! \n");
		return;
	}

	fprintf(file, "%s\n", img->format);
	fprintf(file, "%d %d\n", img->row, img->column);
	fprintf(file, "%d\n", img->maxValue);

	if(strcmp(img->format, "P5") == 0){
		for(rowIndex = 0; rowIndex < img->column; rowIndex++){
	    	fwrite(img->data[rowIndex], sizeof(unsigned char), img->row , file);
    	}
	}
	
	if(strcmp(img->format, "P6") == 0){
		for(rowIndex = 0; rowIndex < img->column; rowIndex++){
	    	for(columnIndex = 0; columnIndex < img->row; columnIndex++){
	    		fwrite(&img->dataRGB[rowIndex][columnIndex].red, sizeof(unsigned char), 1 , file);
	    		fwrite(&img->dataRGB[rowIndex][columnIndex].green, sizeof(unsigned char), 1 , file);
	    		fwrite(&img->dataRGB[rowIndex][columnIndex].blue, sizeof(unsigned char), 1 , file);
	    	}
    	}
	}
	fclose(file);
	return;
}

/*
*	Functia resize este cea care realizarea redimensionarea imaginii(de la diminensiunea initiala la o dimensiune
obtinuta prin aplicarea factorului de scalare).
*/
void resize(image *in, image * out) { 
	unsigned char gaussianKernel[3][3] = {  
								{1, 2, 1} ,   /*  initializers for row indexed by 0 */
								{2, 4, 2} ,   /*  initializers for row indexed by 1 */
								{1, 2, 1}   /*  initializers for row indexed by 2 */
								};
	char magicNum[128];
	sprintf(magicNum, "%s", in->format);
/* seteaza numarul de thread-uri pe care am dori sa ruleze codul, cu observatia ca - daca acest numar este foarte mare - 
*nu garantia acelui numar de theaduri, dar putem obtine maximul posibil.
*/
	omp_set_num_threads(num_threads);

						/*
						*    GRAYSCALE CASE : 
						*/

	if(strcmp(magicNum, "P5") == 0){
		int rowIndex, rowIndex1,columnIndex, columnIndex1;
		int i, sum;
		int noElem = resize_factor * resize_factor;


/* Alocarea matricei output se realizeaza pentru dimensiunile matricei input in matricea corespunzatoare formatului*/
		out->data = (unsigned char **)calloc(in->column , sizeof(unsigned char*));
	    for(i = 0; i < in->column; i++){
	    	out->data[i] = (unsigned char*)calloc(in->row , sizeof(unsigned char));
	    }
/*
*		Pentru cazul in care resize_factor este par se randeaza imagimea prin media aritmetica a pixelor aferenti
*	iar apoi se scaleaza la dimensiunea corespunzatoare.
*/
		if(resize_factor % 2 == 0){
			 #pragma omp parallel for schedule(dynamic) private(rowIndex, columnIndex, rowIndex1, columnIndex1, sum)

			for(rowIndex = 0; rowIndex < (int)(in->column/resize_factor) * resize_factor; rowIndex += resize_factor){
				for(columnIndex = 0; columnIndex < (int)(in->row/resize_factor) * resize_factor; columnIndex += resize_factor){
					sum = 0;

					for(rowIndex1 = rowIndex; rowIndex1 < rowIndex + resize_factor; rowIndex1++){
						for(columnIndex1 = columnIndex; columnIndex1 < columnIndex + resize_factor; columnIndex1++){
							sum += in->data[rowIndex1][columnIndex1];
						}
					}
					out->data[rowIndex/resize_factor][columnIndex/resize_factor] = sum/noElem;
				}
			}
		}else{
/*
*	Pentru cazul in care resize_factor este 3 se randeaza imaginea folosind Kernelul Gaussian. Pentru a aplica
Kernel Gaussian fiecare matrice de 3x3 din imagine este inmultita element cu element cu kernelul.

*/			if(resize_factor == 3){
				#pragma omp parallel for schedule(dynamic) private(rowIndex, columnIndex, rowIndex1, columnIndex1)
				
				for(rowIndex = 0; rowIndex < (int)(in->column/resize_factor) * resize_factor; rowIndex += resize_factor){
					for(columnIndex = 0; columnIndex < (int)(in->row/resize_factor) * resize_factor; columnIndex += resize_factor){
						int sum = 0, rowKernel = 0, columnKernel = 0;

						for(rowIndex1 = rowIndex; rowIndex1 < rowIndex + resize_factor; rowIndex1++){
							for(columnIndex1 = columnIndex; columnIndex1 < columnIndex + resize_factor; columnIndex1++){
								sum += in->data[rowIndex1][columnIndex1] * gaussianKernel[rowKernel][columnKernel];
								columnKernel++;
							}
							columnKernel = 0;
							rowKernel ++;
						}
/* Valoarea pixelului va fi suma impartita la 16 - suma elementelor din kernel.*/
						out->data[rowIndex/resize_factor][columnIndex/resize_factor] = sum/16;
					}
				}
			}
		}
		sprintf(out->format, "%s", in->format);
		out->maxValue = in->maxValue;
		out->row = in->row / resize_factor;
		out->column = in->column / resize_factor;
	}
						/*
						*    RGB CASE : 
						*/

	if(strcmp(in->format, "P6") == 0){
		int rowIndex, rowIndex1,columnIndex, columnIndex1, i;
		int noElem = resize_factor * resize_factor;

/* Alocarea matricei output se realizeaza pentru dimensiunile matricei input in matricea corespunzatoare formatului*/
		out->dataRGB = (PixelRGB **)calloc(in->column , sizeof(PixelRGB*));
		for(i = 0; i < in->column; i++){
			out->dataRGB[i] = (PixelRGB*)calloc(in->row , sizeof(PixelRGB));
		}
/*
*		Pentru cazul in care resize_factor este par se randeaza imagimea prin media aritmetica a pixelor aferenti
*	iar apoi se scaleaza la dimensiunea corespunzatoare.
*/
		if(resize_factor % 2 == 0){
			#pragma omp parallel for schedule(dynamic) private(rowIndex, columnIndex, rowIndex1, columnIndex1)
			
			for(rowIndex = 0; rowIndex < ((int)in->column/resize_factor) * resize_factor; rowIndex += resize_factor){
				for(columnIndex = 0; columnIndex < ((int)in->row/resize_factor) * resize_factor; columnIndex += resize_factor){
					int sumRedPixels = 0;
					int sumGreenPixels = 0;
					int sumBluePixels = 0;
					
					for(rowIndex1 = rowIndex; rowIndex1 < rowIndex + resize_factor; rowIndex1++){
						for(columnIndex1 = columnIndex; columnIndex1 < columnIndex + resize_factor; columnIndex1++){
							sumRedPixels += in->dataRGB[rowIndex1][columnIndex1].red;
							sumGreenPixels += in->dataRGB[rowIndex1][columnIndex1].green; 
							sumBluePixels += in->dataRGB[rowIndex1][columnIndex1].blue;
						}
					}
					out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].red = sumRedPixels/noElem;
					out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].green = sumGreenPixels/noElem;
					out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].blue = sumBluePixels/noElem;
				}
			}
		}else{
/*
*		Pentru cazul in care resize_factor este 3 se randeaza imaginea folosind Kernelul Gaussian. Pentru a aplica
Kernel Gaussian fiecare matrice de 3x3 din imagine este inmultita element cu element cu kernelul.

*/	
			if(resize_factor == 3){
				#pragma omp parallel for schedule(dynamic) private(rowIndex, columnIndex, rowIndex1, columnIndex1)
				
				for(rowIndex = 0; rowIndex < ((int)in->column/resize_factor) * resize_factor; rowIndex += resize_factor){
					for(columnIndex = 0; columnIndex < ((int)in->row/resize_factor) * resize_factor; columnIndex += resize_factor){
						int sumRedPixels = 0;
						int sumGreenPixels = 0;
						int sumBluePixels = 0;
						int rowKernel = 0, columnKernel = 0;
						
						for(rowIndex1 = rowIndex; rowIndex1 < rowIndex + resize_factor; rowIndex1++){
							for(columnIndex1 = columnIndex; columnIndex1 < columnIndex + resize_factor; columnIndex1++){
								sumRedPixels += in->dataRGB[rowIndex1][columnIndex1].red * gaussianKernel[rowKernel][columnKernel];
								sumGreenPixels += in->dataRGB[rowIndex1][columnIndex1].green * gaussianKernel[rowKernel][columnKernel];
								sumBluePixels += in->dataRGB[rowIndex1][columnIndex1].blue * gaussianKernel[rowKernel][columnKernel];
								columnKernel++;
							}
							columnKernel = 0;
							rowKernel ++;
						}
/* Valoarea pixelului va fi suma impartita la 16 - suma elementelor din kernel.*/
						out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].red = sumRedPixels/16;
						out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].green = sumGreenPixels/16;
						out->dataRGB[rowIndex/resize_factor][columnIndex/resize_factor].blue = sumBluePixels/16;
					}
				}
			}
		}
		sprintf(out->format, "%s", in->format);
		out->maxValue = in->maxValue;
		out->row = in->row / resize_factor;
		out->column = in->column / resize_factor;
	}
}
