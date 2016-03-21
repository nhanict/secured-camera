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

#endif // __IMAGE_PROCESSING_H_
