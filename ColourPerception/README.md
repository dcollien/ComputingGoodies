Description of Operation:
=========================

## runServer.c 
Contains the main function which starts an HTTP server for serving colour perception filtered images. This takes an OpenLearning profile name and password (which has admin access to a course) which it uses to authenticate with the OpenLearning API.

## perceptionRequestHandler.c/h
This file handles the actual request as it comes in from the server.

A request comes in with a URL of the forms: 

    /{Cohort-Page-Path}/{profile.name}/{profileHash}/{imageName}/X{x}_Y{y}_Z{z}.{xxx}

or

    /{Cohort-Page-Path}/{profile.name}/{profileHash}/{imageName}/image.{xxx}

Where {..} indicates a replaced field (not literal {}):
- {Cohort-Page-Path} is the path to an OpenLearning course's cohort page, with the /'s replaced with -'s
- {x}, {y} and {z} are the x, y, zoom coordinates for an image
- .{xxx} is the file extension (e.g. .png or .bmp or .jpg)
- {profile.name} is the profile name of the user which is perceiving the image
- {profileHash} is the profile name signed with a shared secret
- {imageName} becomes {imageName}.{xxx}, which is the filename of the image to filter and serve (sourced from under the /images directory in the same folder as where the executable is run)

"servePerceptionImage" uses "get_OLConnection_userCohortInfo" to send a request to OpenLearning's API, to retrieve the values of the "extraData" field which is stored with a user under an enrolment (favourite colour), as well as karma and activity progress.

The favourite colour, karma, activity progress and the user's profile name are sent to "getVisionForUser" to retrieve the filtering calculations. The source image is then loaded up, transformed, served up as a response, and saved to an image cache.

## weakEncrypt.c/h

This file contains the functions which signs the profile name with a shared secret (to verify the URL and prevent profile-name spoofing).

## Helper Libraries

There are also various helper libraries:
- imageTools: for performing image manipulations on multiple image formats using ImageMagick (e.g. get_imageTransform)
- openlearningAPI: for asking questions about OpenLearning users, via the OpenLearning API (e.g. get_OLConnection_userCohortInfo)
- perception: for calculating the colour perception of a user, from the given user data. (e.g. getVisionForUser in vision.c)



Dependent on C Libraries:
========================

- libcurl http://curl.haxx.se/libcurl/ - communicating with OpenLearning
- jansson http://www.digip.org/jansson/ - decoding/encoding JSON data
- libevent http://libevent.org - efficient HTTP server

ImageMagick MagicWand API http://www.imagemagick.org/script/magick-wand.php - creating and transforming images in multiple formats


On OSX ImageMagick can be installed with homebrew:

    brew install imagemagick

On Debian/Ubuntu with apt-get:

    apt-get install libmagickwand-dev


Others can be installed from source: i.e. with: ./configure; make; make install

Compiling:
==========

filteredImageServer/build.sh contains the compile line require to build the perception-filtered image server
this creates a bin/runPerceptionServer executable

Nginx:
======

An Nginx conf file is included which serves images directly from the image cache, and only fallsback on this server if no image has been generated yet

