/*
 *  vision.c
 *  perceptionFilter
 *
 *  Created by Richard Buckland on 11/02/11.
 *  Copyright 2011 Licensed under Creative Commons SA-BY-NC 3.0. All rights reserved.
 *
 *  Modified by David Collien
 *
 */

#include "vision.h"
#include <string.h>
#include <stdio.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define KARMA_SCALE_CONST 0.01
#define PROGRESS_SCALE_CONST 0.03

#define CAP 0.8

#define KARMA_MODE_CHANGE 10
#define KARMA_MODE_PERC 0.05

#define PROGRESS_MODE_CHANGE 20
#define PROGRESS_MODE_PERC 0.025

#define HASH_OFFSET 2166136261
#define HASH_PRIME 16777619

#define HASH_BITS 5
#define HASH_MASK ((1<<HASH_BITS)-1)

static void setFav (int favColor, double channelValue, vision *vision);
static void generateScale (const char *profileName, vision *vision);
static void capAllButFav (int favColor, vision *vision);
static void multiplyVisionBy (double scalar, vision *vision);
static double getFav (int favColor, vision *vision);

int getColorFromString (const char *colorString) {
	if (strncmp (colorString, "Red", strlen ("Red")) == 0) {
		return COLOR_RED;
	} else if (strncmp (colorString, "Green", strlen ("Green")) == 0) {
		return COLOR_GREEN;
	} else if (strncmp (colorString, "Blue", strlen ("Blue")) == 0) {
		return COLOR_BLUE;
	} else if (strncmp (colorString, "Cyan", strlen ("Cyan")) == 0) {
		return COLOR_CYAN;
	} else if (strncmp (colorString, "Yellow", strlen ("Yellow")) == 0) {
		return COLOR_YELLOW;
	} else if (strncmp (colorString, "Magenta", strlen ("Magenta")) == 0) {
		return COLOR_MAGENTA;
	}
	return COLOR_NONE;
}

vision getVisionForUser (const char *profileName, int karma, int favColor, int progress) {
	vision scale;

	vision karmaVision;
	vision progressVision;
	vision totalVision;

	double favChannel;
	double multiplier;

	karma = MAX(0, karma);
	progress = MAX(0, progress);
	favColor = MAX(0, favColor);

	printf ("\n%s\n", profileName);
	printf ("Generating Vision: karma = %d, favColor = %d, progress = %d ...\n", karma, favColor, progress);
	printf ("  %s's scale:\n", profileName);
	generateScale (profileName, &scale);

	printf ("  r: %lf\n", scale.r);
	printf ("  g: %lf\n", scale.g);
	printf ("  b: %lf\n", scale.b);
	printf ("  c: %lf\n", scale.gb);
	printf ("  m: %lf\n", scale.rb);
	printf ("  y: %lf\n", scale.rg);

	printf ("\n Karma Component Calculation...\n");
	if (karma <= KARMA_MODE_CHANGE) {
		printf ("  Karma under threshold: %d/%d\n", karma, KARMA_MODE_CHANGE);
		noColorVision (&karmaVision);
		favChannel = karma * KARMA_MODE_PERC;
	} else {
		printf ("  Karma over threshold: %d/%d\n", karma, KARMA_MODE_CHANGE);
		karmaVision = scale;
		multiplier = (KARMA_SCALE_CONST * karma);
		multiplyVisionBy (multiplier, &karmaVision);
		printf ("  Multiplied scale by: %lf\n", multiplier);
		favChannel = getFav (favColor, &karmaVision) + 0.5;
	}
	setFav (favColor, favChannel, &karmaVision);
	printf ("  Set fav color to %lf\n", favChannel);

	printf ("\n Karma Component Result:\n");
	printf ("  r = %lf\n", karmaVision.r);
	printf ("  g = %lf\n", karmaVision.g);
	printf ("  b = %lf\n", karmaVision.b);
	printf ("  c = %lf\n", karmaVision.gb);
	printf ("  m = %lf\n", karmaVision.rb);
	printf ("  y = %lf\n", karmaVision.rg);

	printf ("\n Progress Component Calculation...\n");
	if (progress <= PROGRESS_MODE_CHANGE) {
		printf ("  Progress under threshold: %d/%d\n", progress, PROGRESS_MODE_CHANGE);
		noColorVision (&progressVision);
		favChannel = progress * PROGRESS_MODE_PERC;
		setFav (favColor, favChannel, &progressVision);
		printf ("  Set fav color to %lf\n", favChannel);
	} else {
		printf ("  Progress over threshold: %d/%d\n", progress, PROGRESS_MODE_CHANGE);
		progressVision = scale;
		multiplier = PROGRESS_SCALE_CONST * progress;
		multiplyVisionBy (multiplier, &progressVision);
		printf ("  Multiplied scale by: %lf\n", multiplier);
	}
	printf ("\n Progress Component Result:\n");
	printf ("  r = %lf\n", progressVision.r);
	printf ("  g = %lf\n", progressVision.g);
	printf ("  b = %lf\n", progressVision.b);
	printf ("  c = %lf\n", progressVision.gb);
	printf ("  m = %lf\n", progressVision.rb);
	printf ("  y = %lf\n", progressVision.rg);

	printf ("\n Adding Karma to Progress (except Magenta)...\n");

	totalVision.r = karmaVision.r + progressVision.r;
	totalVision.g = karmaVision.g + progressVision.g;
	totalVision.b = karmaVision.b + progressVision.b;
	totalVision.gb = karmaVision.gb + progressVision.gb;
	totalVision.rb = karmaVision.rb; // No Progress Magenta
	totalVision.rg = karmaVision.rg + progressVision.rg;

	printf (" Capping at %lf except for fav color\n", CAP);

	capAllButFav (favColor, &totalVision);

	printf ("\n Total Vision Result:\n");
	printf ("  r = %lf\n", totalVision.r);
	printf ("  g = %lf\n", totalVision.g);
	printf ("  b = %lf\n", totalVision.b);
	printf ("  c = %lf\n", totalVision.gb);
	printf ("  m = %lf\n", totalVision.rb);
	printf ("  y = %lf\n", totalVision.rg);

	return totalVision;
}

void noColorVision (vision *v) {
   v->r = 0;
   v->g = 0;
   v->b = 0;
   v->rb = 0;
   v->gb = 0;
   v->rg = 0;
   v->resolution = 1;
}

static void capAllButFav (int favColor, vision *vision) {
	double rCap, gCap, bCap;
	double gbCap, rbCap, rgCap;

	rCap = CAP;
	gCap = CAP;
	bCap = CAP;

	gbCap = CAP;
	rbCap = CAP;
	rgCap = CAP;

	switch (favColor) {

		case COLOR_RED:
			rCap = 1.0; break;
		case COLOR_GREEN:
			gCap = 1.0; break;
		case COLOR_BLUE:
			bCap = 1.0; break;

		case COLOR_CYAN:
			gbCap = 1.0; break;
		case COLOR_MAGENTA:
			rbCap = 1.0; break;
		case COLOR_YELLOW:
			rgCap = 1.0; break;

		default:
			break;
	}

	vision->r  = MIN(rCap, vision->r);
	vision->g  = MIN(gCap, vision->g);
	vision->b  = MIN(bCap, vision->b);

	vision->gb = MIN(gbCap, vision->gb);
	vision->rb = MIN(rbCap, vision->rb);
	vision->rg = MIN(rgCap, vision->rg);
}

static void multiplyVisionBy (double scalar, vision *vision) {
	vision->r *= scalar;
	vision->g *= scalar;
	vision->b *= scalar;
	vision->gb *= scalar;
	vision->rb *= scalar;
	vision->rg *= scalar;
}

static void generateScale (const char *profileName, vision *vision) {
	int numChars = strlen (profileName);
	int i;
	unsigned int hash = HASH_OFFSET;

	char r, g, b, c, m, y;

	for (i = 0; i != numChars; ++i) {
		hash ^= profileName[i];
		hash *= HASH_PRIME;
	}

	r = hash & HASH_MASK;
	hash >>= HASH_BITS;
	g = hash & HASH_MASK;
	hash >>= HASH_BITS;
	b = hash & HASH_MASK;
	hash >>= HASH_BITS;
	c = hash & HASH_MASK;
	hash >>= HASH_BITS;
	m = hash & HASH_MASK;
	hash >>= HASH_BITS;
	y = hash & HASH_MASK;

	vision->r = r/(double)HASH_MASK;
	vision->g = g/(double)HASH_MASK;
	vision->b = b/(double)HASH_MASK;
	vision->gb = c/(double)HASH_MASK;
	vision->rb = m/(double)HASH_MASK;
	vision->rg = y/(double)HASH_MASK;
}

static double getFav (int favColor, vision *vision) {
	double channelValue;
	switch (favColor) {
		case COLOR_RED:
			channelValue = vision->r; break;
		case COLOR_GREEN:
			channelValue = vision->g; break;
		case COLOR_BLUE:
			channelValue = vision->b; break;
		case COLOR_CYAN:
			channelValue = vision->gb; break;
		case COLOR_MAGENTA:
			channelValue = vision->rb; break;
		case COLOR_YELLOW:
			channelValue = vision->rg; break;
		default:
			channelValue = 0.0;
			break;
	}

	return channelValue;
}

static void setFav (int favColor, double channelValue, vision *vision) {
	switch (favColor) {
		case COLOR_RED:
			vision->r = channelValue; break;
		case COLOR_GREEN:
			vision->g = channelValue; break;
		case COLOR_BLUE:
			vision->b = channelValue; break;
		case COLOR_CYAN:
			vision->gb = channelValue; break;
		case COLOR_MAGENTA:
			vision->rb = channelValue; break;
		case COLOR_YELLOW:
			vision->rg = channelValue; break;
		default:
			break;
	}
}
