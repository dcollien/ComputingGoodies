#include "vision.h"
#include "rgb.h"
#include "perception.h"

void perceptionTransformer (
	// coordinates
	const int x,
	const int y,

	// normalised pixel value [0.0 .. 1.0]
	double *r,
	double *g,
	double *b,

	void *data
) {
	vision *userVision = (vision *)data;
	rgb pixel, transformedPixel;

	pixel.red = (int)((*r) * MAX_COLOR_VALUE);
	pixel.green = (int)((*g) * MAX_COLOR_VALUE);
	pixel.blue = (int)((*b) * MAX_COLOR_VALUE);

	transformedPixel = perceive (pixel, *userVision);

	*r = transformedPixel.red/(double)MAX_COLOR_VALUE;
	*g = transformedPixel.green/(double)MAX_COLOR_VALUE;
	*b = transformedPixel.blue/(double)MAX_COLOR_VALUE;
}
