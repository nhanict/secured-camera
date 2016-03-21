#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/stat.h>

__u8* RGB2YUYV(__u8 *input, int w, int h){
	__u8* yuyv = (__u8*)malloc(w*h*2);
	int i;
	for(i=0; i<w*h; i+=2){
		__u8 y0, u, y1, v, r0, g0, b0, r1, g1, b1;
		memcpy(&r0, input + 3*i	, (size_t)1);
		memcpy(&g0 , input + 3*i+1, 1);
		memcpy(&b0, input + 3*i+2, 1);
		memcpy(&r1, input + 3*i+3, 1);
		memcpy(&g1 , input + 3*i+4, 1);
		memcpy(&b1, input + 3*i+5, 1);

		y0 = (__u8)((299*(int)r0 + 587*(int)g0 + (int)b0*0)/1000);
		u = (__u8)((-169*(int)r0 - 331*(int)g0 + 499*(int)b0)/1000 + 128);
		v = (__u8)((4990*r0 - 4180*g0 - 813*b0)/10000 + 128);
		y1 = (__u8)((299*r1 + 587*g1 + b1*0)/1000);

		memcpy(yuyv + 2*i, &y0, 1);
		memcpy(yuyv + 2*i+1, &u, 1);
		memcpy(yuyv + 2*i+2, &y1, 1);
		memcpy(yuyv + 2*i+3, &v, 1);
	}
	return yuyv;

}

__u8* BGR2YUYV(__u8 *input, int w, int h){
	__u8* yuyv = (__u8*)malloc(w*h*2);
	int i;
	for(i=0; i<w*h; i+=2){
		__u8 y0, u, y1, v, r0, g0, b0, r1, g1, b1;
		memcpy(&b0, input + 3*i	, 1);
		memcpy(&g0 , input + 3*i+1, 1);
		memcpy(&r0, input + 3*i+2, 1);
		memcpy(&b1, input + 3*i+3, 1);
		memcpy(&g1 , input + 3*i+4, 1);
		memcpy(&r1, input + 3*i+5, 1);

		y0 = (__u8)((299*(int)r0 + 587*(int)g0 + (int)b0*0)/1000);
		u = (__u8)((-169*(int)r0 - 331*(int)g0 + 499*(int)b0)/1000 + 128);
		v = (__u8)((4990*r0 - 4180*g0 - 813*b0)/10000 + 128);
		y1 = (__u8)((299*r1 + 587*g1 + b1*0)/1000);

		memcpy(yuyv + 2*i, &y0, 1);
		memcpy(yuyv + 2*i+1, &u, 1);
		memcpy(yuyv + 2*i+2, &y1, 1);
		memcpy(yuyv + 2*i+3, &v, 1);
	}
	return yuyv;

}

__u8* YUYV2RGB(__u8 *input, int w, int h){
	__u8* rgb = (__u8*)malloc(w*h*3);
	int i;
	for(i=0; i<w*h; i+=2){
		__u8 y0, u, y1, v, r, g, b;
		memcpy(&y0, input + 2*i	, 1);
		memcpy(&u , input + 2*i+1, 1);
		memcpy(&y1, input + 2*i+2, 1);
		memcpy(&v , input + 2*i+3, 1);
	   r = (__u8)((1000*(int)y0 + 1402 * (((int)v) - 128))/1000);
	   g = (__u8)((1000*(int)y0 - 344 * (((int)u)-128) - 714 * (((int)v)-128))/1000);
	   b = (__u8)((1000*(int)y0 + 1772 * (((int)u)-128))/1000);
		if (r < 0) r = 0;
	   if (g < 0) g = 0;
	   if (b < 0) b = 0;
	   if (r > 255) r = 255;
	   if (g > 255) g = 255;
	   if (b > 255) b = 255;
		memcpy(rgb + 3*i, &r, 1);
		memcpy(rgb + 3*i+1, &g, 1);
		memcpy(rgb + 3*i+2, &b, 1);
		//
	   r = (__u8)((1000*(int)y1 + 1402 * (((int)v) - 128))/1000);
	   g = (__u8)((1000*(int)y1 - 344 * (((int)u)-128) - 714 * (((int)v)-128))/1000);
	   b = (__u8)((1000*(int)y1 + 1772 * (((int)u)-128))/1000);
		if (r < 0) r = 0;
	   if (g < 0) g = 0;
	   if (b < 0) b = 0;
	   if (r > 255) r = 255;
	   if (g > 255) g = 255;
	   if (b > 255) b = 255;

		memcpy(rgb + 3*i+3, &r, 1);
		memcpy(rgb + 3*i+4, &g, 1);
		memcpy(rgb + 3*i+5, &b, 1);
	}
	return rgb;
}

void _mkdir(const char *dir) {
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp),"%s",dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
			tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++)
			if(*p == '/') {
					*p = 0;
					mkdir(tmp, S_IRWXU);
					*p = '/';
			}
	mkdir(tmp, S_IRWXU);
}

#endif //_UTILS_H_
