#ifndef __IMAGE_PROCESSING_H_
#define __IMAGE_PROCESSING_H_

typedef unsigned char uint8;


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

uint8 **read_memory2image(const uint8 *input, int rows, int cols){
	int i;
	uint8 **out = image_array_alloc(rows, cols);
	for(i=0; i<rows; i++){
		memcpy(out[i], input+i*cols, cols*sizeof(uint8));
	}
	return out;

}

#endif // __IMAGE_PROCESSING_H_
