#ifndef IMAGE_INIT_H
#define IMAGE_INIT_H

typedef void (*pixelOperator)(
	const int x,
	const int y,
	double *r,
	double *g,
	double *b,
	void *data
);

void init_imageTools (void);
void deinit_imageTools (void);

#endif
