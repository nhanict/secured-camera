#ifndef __IMAGE_PROCESSING_H_
#define __IMAGE_PROCESSING_H_

typedef unsigned char uint8;


const uint8 mask3_vert[3][3] = {{0, 1, 0},
									 {0, 1, 0},
									 {0, 1, 0}};
const uint8 mask3_hori[3][3] = {{0, 0, 0},
									 {1, 1, 1},
									 {0, 0, 0}};
const uint8 mask3_cros[3][3] = {{1, 0, 1},
									 {0, 1, 0},
									 {1, 0, 1}};
const uint8 mask3_plus[3][3] = {{0, 1, 0},
									 {1, 1, 1},
									 {0, 1, 0}};
const uint8 mask3_full[3][3] = {{1, 1, 1},
									 {1, 1, 1},
									 {1, 1, 1}};

uint8 **image_array_alloc(int rows, int cols){
	int i;
	uint8 **out =(uint8**) malloc(rows*sizeof(uint8 *));
	for(i=0; i<rows; i++){
		out[i] = (uint8*)malloc(cols*sizeof(uint8));
		if(out[i] == '\0')
			printf("Failed to allocate memory\n");
	}
	return(out);
}

void image_array_free(uint8 **image, int rows){
	int i;
	for(i=0; i<rows; i++){
		free(image[i]);
	}
	free(image);
}

void save_as_number_image(const char *text_file, uint8 **image, int rows, int cols){
	int i,j;
	FILE *file = fopen(text_file, "wb");
	if(file == NULL){
		printf("Error Cannot create output file\n");
		return;
	} else {
		for (i=0; i<rows; i++){
			fwrite(image[i], cols, sizeof(uint8), file);
			fputc('\n', file);
		}
	}
	fclose(file);
}

uint8 **convertMemory22DImage(const uint8 *input, int rows, int cols){
	int i;
	uint8 **out = image_array_alloc(rows, cols);
	for(i=0; i<rows; i++){
		memcpy(out[i], input+i*cols, cols*sizeof(uint8));
	}
	return out;
}

uint8 *convert2DImage2Memory(const uint8** input, int rows, int cols){
	int i;
	uint8 *out = (uint8 *)malloc(rows*cols*sizeof(uint8));
	for(i=0; i<rows; i++){
		memcpy(&out[i*cols], input[i], cols*sizeof(uint8));
	}
	return out;
}

/********************************************************************
 * 		erosion()
 * 		perform the erosion operation.
 * 	@param input: 2D input image
 *	@param value: the pixel value at which erosion process is applied.
 *	@param threshold: minimum threshold of neighbor pixel that is background pixel.
 *	@param rows: number of rows.
 *	@param cols: number of cols.
 ********************************************************************/
uint8 **erosion(const uint8 **input, int value, int threshold, int rows, int cols){
	uint8 **output = image_array_alloc(rows, cols);
	int i, j, a, b, count;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++)
			output[i][j] = input[i][j];
	for(i=1; i<rows-1; i++)
		for(j=1; j<cols-1; j++){
			if(input[i][j] == value){
				count=0;
				for(a=-1; a<=1; a++){
					for(b=-1; b<=1; b++){
						if(input[i+a][j+b] == 0)
							count++;
					}
				}
				if(count > threshold)
					output[i][j] = 0;
			}
		}
	// fix_edges()
}

/***********************************************************
 * 		mask_erosion()
 * 	Performs the dilation operation with mask.
 * 	@param input: 2D input image
 * 	@param mask: 2D input mask
 *	@param value: the pixel value at which erosion process is applied.
 *	@param threshold: minimum threshold of neighbor pixel that is background pixel.
 *	@param rows: number of rows.
 *	@param cols: number of cols.
 ***********************************************************/
uint8 **mask_erosion(const uint8 **input, const uint8 **mask, int value, int threshold, int rows, int cols){
	uint8 **output = image_array_alloc(rows, cols);
	int i, j, a, b, count;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++)
			output[i][j] = input[i][j];
	for(i=1; i<rows-1; i++)
		for(j=1; j<cols-1; j++){
			if(input[i][j] == value){
				count=0;
				for(a=-1; a<=1; a++){
					for(b=-1; b<=1; b++){
						if(input[i+a][j+b] == 0)
							count++;
					}
				}
				if(count > threshold)
					output[i][j] = 0;
			}
		}
	// fix_edges()
}

/***********************************************************
 * 		dilation()
 * 	Performs the dilation operation.
 * 	@param input: 2D input image
 *	@param value: the pixel value at which Dilation process is applied.
 *	@param threshold: minimum threshold of neighbor pixel that is background pixel.
 *	@param rows: number of rows.
 *	@param cols: number of cols.
 ***********************************************************/
uint8 **dilation(const uint8 **input, int value, int threshold, int rows, int cols){
	uint8 **output = image_array_alloc(rows, cols);
	int i, j, a, b, count;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++)
			output[i][j] = input[i][j];
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++){
			if(input[i][j] == 0){
				count = 0;
				for(a=-1; a<=1; a++)
					for(b=-1; b<=1; b++)
						if(input[i+a][j+b] == value)
							count++;
				if(count > threshold)
					output[i][j] = value;
			}
		}

	// fix_edges()
}

/***********************************************************
 * 		mask_dilation()
 * 	Performs the dilation operation with mask.
 * 	@param input: 2D input image
 * 	@param mask: 2D input mask
 *	@param value: the pixel value at which Dilation process is applied.
 *	@param threshold: minimum threshold of neighbor pixel that is background pixel.
 *	@param rows: number of rows.
 *	@param cols: number of cols.
 ***********************************************************/
uint8 **mask_dilation(const uint8 **input, const uint8 **mask, int value, int threshold, int rows, int cols){
	uint8 **output = image_array_alloc(rows, cols);
	int i, j, a, b, max;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++)
			output[i][j] = input[i][j];
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++){
				for(a=-1; a<=1; a++)
					for(b=-1; b<=1; b++)
						if(input[i+a][j+b] == value)
							max++;
				if(max > threshold)
					output[i][j] = value;

		}

	// fix_edges()
}



#endif // __IMAGE_PROCESSING_H_
