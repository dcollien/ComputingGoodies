#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rgb.h"
#include "vision.h"
#include "perception.h"
#include "testPerception.h"

static int areSameColor (rgb seen, rgb expected);

int main (int argc, const char * argv[]) {
   testPerception();
   printf ("All tests passed.  You are AWESOME!\n");
   return EXIT_SUCCESS;
}

void testPerception (void) {
   rgb actual;
   rgb seen;   
   rgb expected;
   vision viewer;
   noColorVision (&viewer);
   
   {
      // averaging to gray when no color perception
      noColorVision (&viewer);
      actual.red = 50;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // no rounding when is a multiple of resolution
      noColorVision (&viewer);
      viewer.resolution = 20;
      
      actual.red = 50;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
 
   {
      // no rounding up
      noColorVision (&viewer);
      viewer.resolution = 50;
      
      actual.red = 50;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 100;            
      expected.green = 100;  
      expected.blue = 100;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // rounding down
      noColorVision (&viewer);
      viewer.resolution = 128;
      
      actual.red = 50;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 128;            
      expected.green = 128;  
      expected.blue = 128;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // not rounding up on midpoint
      noColorVision (&viewer);
      viewer.resolution = 40;
      
      actual.red = 50;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 120;            
      expected.green = 120;  
      expected.blue = 120;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // rounding down just under midpoint
      noColorVision (&viewer);
      viewer.resolution = 40;
      
      actual.red = 49;
      actual.green = 120;
      actual.blue = 250;
      
      expected.red = 120;            
      expected.green = 120;  
      expected.blue = 120;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // perfect red vision
      noColorVision (&viewer);
      viewer.r = 1.0;
      
      actual.blue = 40;
      actual.green = 130;
      actual.red = 250;
      
      expected.red = 120 + 100;            
      expected.green = 100;  
      expected.blue = 100;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial red vision
      // red dominates
      noColorVision (&viewer);
      viewer.r = 0.5;
      
      actual.red  =  250;
      actual.green = 130;
      actual.blue   = 40;
      
      expected.red = 120+60;            
      expected.green = 120;  
      expected.blue = 120;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial red vision, no red in picture
      // green dominates
      noColorVision (&viewer);
      viewer.r = 0.5;
      
      actual.blue = 130;
      actual.red = 40;
      actual.green = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial red vision, no red in picture
      // blue dominates
      noColorVision (&viewer);
      viewer.r = 0.5;
      
      actual.red = 130;
      actual.green = 40;
      actual.blue = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // perfect green vision
      noColorVision (&viewer);
      viewer.g = 1.0;
      
      actual.blue = 40;
      actual.green = 250;
      actual.red = 130;
      
      expected.red = 100;            
      expected.green = 120 + 100;  
      expected.blue = 100;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial green vision
      // green dominates
      noColorVision (&viewer);
      viewer.g = 0.5;
      
      actual.green =  250;
      actual.blue  = 130;
      actual.red   = 40;
      
      expected.red = 120;            
      expected.green = 120+60;  
      expected.blue = 120;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial green vision, no green in picture
      // red dominates
      noColorVision (&viewer);
      viewer.g = 0.5;
      
      actual.blue = 130;
      actual.green = 40;
      actual.red = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial green vision, no green in picture
      // blue dominates
      noColorVision (&viewer);
      viewer.g = 0.5;
      
      actual.red = 40;
      actual.green = 130;
      actual.blue = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {
      // perfect blue vision
      noColorVision (&viewer);
      viewer.b = 1.0;
      
      actual.red = 40;
      actual.green = 130;
      actual.blue = 250;
      
      expected.red = 100;            
      expected.green = 100;  
      expected.blue = 120 + 100;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial blue vision
      noColorVision (&viewer);
      viewer.b = 0.5;
      
      actual.red = 130;
      actual.green = 40;
      actual.blue = 250;
      
      expected.red = 120;            
      expected.green = 120;  
      expected.blue = 60 + 120;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial blue vision, no blue in picture
      // red dominates
      noColorVision (&viewer);
      viewer.b = 0.5;
      
      actual.blue = 130;
      actual.green = 40;
      actual.red = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // partial blue vision, no blue in picture
      // green dominates
      noColorVision (&viewer);
      viewer.b = 0.5;
      
      actual.red = 130;
      actual.blue = 40;
      actual.green = 250;
      
      expected.red = 140;            
      expected.green = 140;  
      expected.blue = 140;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   
   {      
      // half purple vision
      // with purple in picture
      noColorVision (&viewer);
      viewer.rb = 0.5;
      
      actual.red  = 250;
      actual.green = 40;
      actual.blue = 130;
      
      // total color = 420
      // actual 90 purple each for red and blue
      // see 45 each r b (50% vision)
      // remainder = 420 - 90 = 330
      // remainder 330 = 110 each rgb
      expected.red = 110+45;            
      expected.green = 110;  
      expected.blue = 110+45;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // half purple vision
      // 25% blue vision
      // with no purple in picture
      noColorVision (&viewer);
      viewer.rb = 0.5;
      viewer.b = 0.25;
      
      actual.blue  = 270;
      actual.red = 120;
      actual.green = 210;
      
      // total color = 600
      // actual 0 purple , blue = 60
      // see 0 each r b (50% vision)
      // see 15 b (25% vision)
      // remainder = 600-15 = 585
      // remainder 585/3 = 195 each rgb
      expected.red = 195;            
      expected.green = 195;  
      expected.blue = 195+15;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
   
   {      
      // half purple vision
      // 25% blue vision
      // with purple and blue in picture
      noColorVision (&viewer);
      viewer.rb = 0.5;
      viewer.b = 0.25;
      
      actual.blue  = 270;
      actual.green = 120;
      actual.red   = 210;
      
      // total color = 600
      // actual 0 purple = 90, blue = 60
      // see 45 each r b (50% vision)
      // see 15 b (25% vision)
      // remainder = 600-15-45-45 = 495
      // remainder 495/3 = 165 each rgb
      expected.red = 165+45;            
      expected.green = 165;  
      expected.blue = 165+15+45;
      
      seen = perceive (actual, viewer);
      
      assert (areSameColor (seen, expected));   
   }
}


static int areSameColor (rgb seen, rgb expected) {
   int sameColor = (
      (seen.red==expected.red) &&
      (seen.green==expected.green) &&
      (seen.blue==expected.blue)
   );
   
   if (!sameColor) {
      printf ("\n    seen is ");
      showColor (seen);
      printf ("\nexpected is ");
      showColor (expected);
      printf ("\n");
   }
   
   return sameColor;
}