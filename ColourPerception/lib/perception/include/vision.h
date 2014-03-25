/*
 *  vision.h
 *  perceptionFilter
 *
 *  Created by Richard Buckland on 11/02/11.
 *  Copyright 2011 Licensed under Creative Commons SA-BY-NC 3.0. 
 *  Some rights reserved.
 *
 *  Modified by David Collien
 */

#define COLOR_RED     0
#define COLOR_GREEN   1
#define COLOR_BLUE    2
#define COLOR_CYAN    3
#define COLOR_MAGENTA 4
#define COLOR_YELLOW  5

#define COLOR_NONE 6

typedef struct vision {
   double r;
   double g;
   double b; 
   double rb;  // magenta
   double gb;  // cyan
   double rg;  // yellow 
   double resolution;   
} vision;

void noColorVision (vision *v);

vision getVisionForUser (const char *profileName, int karma, int favColor, int progress);


int getColorFromString (const char *colorString);
