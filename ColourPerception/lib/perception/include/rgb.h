/*
 *  rgb.h
 *  perceptionFilter
 *
 *  Created by Richard Buckland on 11/02/11.
 *  Copyright 2011 Licensed under Creative Commons SA-BY-NC 3.0. 
 *  Some rights reserved.
 *
 */

#define MAX_COLOR_VALUE 255
#define NUM_COLORS 3

typedef struct _rgb {
   int red;
   int green;
   int blue;
} rgb;

/*
typedef struct _components {
   int r;
   int g;
   int b;
   int rg;
   int rb;
   int gb;
   int white;
} components;

components rgbToComponents (rgb color);
rgb componentsToRGB (components channels);

void showColor (rgb color);
*/
