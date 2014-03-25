/*
 *  color.c
 *  perceptionFilter
 *
 *  Created by Richard Buckland on 11/02/11.
 *  Copyright 2011 Licensed under Creative Commons SA-BY-NC 3.0. All rights reserved.
 *
 */

#include <stdio.h>
#include <assert.h>
#include "rgb.h"

static int largest (int a, int b, int c);
static int smallest (int a, int b, int c);

components rgbToComponents (rgb color) {
   components channels;
   
   channels.r  = 0;
   channels.g  = 0;
   channels.b  = 0;
   channels.rg = 0;
   channels.gb = 0;
   channels.rb = 0;

   channels.white = smallest (color.red, color.green, color.blue);
   
   int primary = largest (color.red, color.green, color.blue);

   if (color.red == primary) {
      // red is dominant color
      channels.rg = color.green - channels.white;
      channels.rb = color.blue - channels.white;
      channels.r = color.red - channels.rg - channels.rb;
      
   } else if (color.green == primary) {
      // green is dominant color
      channels.rg = color.red - channels.white;
      channels.gb = color.blue - channels.white;
      channels.g = color.green - channels.rg - channels.gb;
      
      
   } else {
      // blue is dominant color
      channels.rb = color.red - channels.white;
      channels.gb = color.green - channels.white;
      channels.b = color.blue - channels.rb - channels.gb;
   }
   
   assert (channels.r >= 0);
   assert (channels.g >= 0);
   assert (channels.b >= 0);
   assert (channels.rb >= 0);
   assert (channels.rg >= 0);
   assert (channels.gb >= 0);

   assert (channels.white <= MAX_COLOR_VALUE);
   
   return channels;
}

static int largest (int a, int b, int c) {
   int result;
   if ((a >= b) & (a >= c)) {
      result = a;
   } else if (b>=c) {
      result = b;
   } else {
      result = c;
   }   
   return result;
}

static int smallest (int a, int b, int c) {
   int result;
   if ((a <= b) & (a <= c)) {
      result = a;
   } else if (b<=c) {
      result = b;
   } else {
      result = c;
   }   
   return result;
}

rgb componentsToRGB (components channels) {
   rgb color;
   
   color.red   = channels.r + channels.rg + channels.rb + channels.white;
   color.green = channels.g + channels.rg + channels.rb + channels.white;
   color.blue  = channels.b + channels.rg + channels.rb + channels.white;
   
   assert ((color.red >= 0)   && (color.red   <= MAX_COLOR_VALUE));
   assert ((color.green >= 0) && (color.green <= MAX_COLOR_VALUE));
   assert ((color.blue >= 0)  && (color.blue  <= MAX_COLOR_VALUE));
   
   return color;
}

void showColor (rgb color) {

   printf ("(%d-%2x, %d-%2x, %d-%2x)", 
           color.red,   color.red,
           color.green, color.green,
           color.blue,  color.blue);
}