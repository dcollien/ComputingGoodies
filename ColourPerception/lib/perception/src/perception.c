/*
 *  perception.c
 *  perceptionFilter
 *
 *  Created by Richard Buckland on 11/02/11.
 *  Copyright 2011 Licensed under Creative Commons SA-BY-NC 3.0. 
 *  Some rights reserved.
 *
 */

#include <stdio.h>
#include <assert.h>
#include "rgb.h"
#include "vision.h"
#include "perception.h"

//static int deRez (int x, int resolution);

rgb perceive (rgb actual, vision viewer) {
   
   rgb seen;
   seen.red = 0;
   seen.green = 0;
   seen.blue = 0;
   
   // scale the primary and secondary colors by viewer perception
   int primary = 0;
   int secondary = 0;
   // here we unroll the ifs: faster but has duplication      
   if ((actual.red >= actual.green) && (actual.red >= actual.blue)) {
      // red is dominant color
      if (actual.green >= actual.blue) {
         // r >= g >= b
         primary   = (actual.red - actual.green) * viewer.r;
         secondary = (actual.green - actual.blue) * viewer.rg;
         seen.red   = primary + secondary;
         seen.green = secondary;
      } else {
         // r >= b >= g
         primary   = (actual.red - actual.blue) * viewer.r;
         secondary = (actual.blue - actual.green) * viewer.rb;
         seen.red   = primary + secondary;
         seen.blue  = secondary;
      }
         
   } else if (actual.green >= actual.blue) {
      // green is dominant color
      
      if (actual.red >= actual.blue) {
         // g >= r >= b
         primary   = (actual.green - actual.red) * viewer.g;
         secondary = (actual.red - actual.blue) * viewer.rg;
         seen.green = primary + secondary;
         seen.red   = secondary;
      } else {
         // g >= b >= r
         primary   = (actual.green - actual.blue) * viewer.g;
         secondary = (actual.blue - actual.red) * viewer.gb;
         seen.green = primary + secondary;
         seen.blue  = secondary;
      }
   
      
   } else {
   // blue is dominant color
      if (actual.red >= actual.green) {
         // b >= r >= g
         primary   = (actual.blue - actual.red) * viewer.b;
         secondary = (actual.red - actual.green) * viewer.rb;
         seen.blue  = primary + secondary;
         seen.red   = secondary;
      } else {
         // b >= g >= r
         primary   = (actual.blue - actual.green) * viewer.b;
         secondary = (actual.green - actual.red) * viewer.gb;
         seen.blue  = primary + secondary;
         seen.green   = secondary;
      }      
   }
   
   assert (primary >= 0);
   assert (secondary >= 0);
   
   // allocate remaining light to gray
   int totalLight = actual.red + actual.green + actual.blue;
   int undifferentiated = totalLight - primary - (secondary*2);
   assert (undifferentiated >= 0);
   int white = undifferentiated / NUM_COLORS;
   assert (white <= MAX_COLOR_VALUE);
   
   //printf ("before adding white\n");
   //showColor (seen);
   //printf ("\n");
   
   seen.red   += white;
   seen.green += white;
   seen.blue  += white;
   
   // round to nearest multiple of resolution
   
   //printf ("after adding white\n");
   //showColor (seen);
   //printf ("\n");

   /*
   seen.red   = deRez (seen.red,   viewer.resolution);
   seen.green = deRez (seen.green, viewer.resolution);
   seen.blue  = deRez (seen.blue,  viewer.resolution);
   */
   
   //printf ("after derezzing\n");
   //showColor (seen);
   //printf ("\n");
   
   assert ((seen.red   >=0) && (seen.red   <= MAX_COLOR_VALUE));
   assert ((seen.green >=0) && (seen.green <= MAX_COLOR_VALUE));
   assert ((seen.blue  >=0) && (seen.blue  <= MAX_COLOR_VALUE));
   
   return seen;
}

// returns x rounded to the nearest multiple of resolution
/* static int deRez (int x, int resolution) {
   assert (resolution > 0);
   int base = x + (resolution/2);
   int surplus = base % resolution;
   return (base - surplus);
}
*/

/*
static int deRez (int x, int resolution) {
   assert (resolution > 0);
   int surplus = x % resolution;
   return (x - surplus);
}
*/
