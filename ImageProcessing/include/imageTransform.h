#ifndef IMAGE_TRANSFORM_H
#define IMAGE_TRANSFORM_H

#include "imageInit.h"

// Transform an image according to imageParams,
// by passing each pixel through the transformer function
unsigned char *get_imageTransform (
	pixelOperator transformer,
	unsigned char *image,
	size_t imageSize,
	size_t *newImageSize,
	void *imageParams
);

#endif
