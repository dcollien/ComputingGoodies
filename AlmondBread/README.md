AlmondBread Mandelbrot Server
-----------------------------

Listens on port 8081 by default. Serves images of the format:
    /X<some x value>_Y<some y value>_Z<some zoom value>.png

e.g.

    /X-0.5_Y0.0_Z8.png


Color Maps are located in ../colormaps/ relative to the execution path

Images are cached as files in ../imageCache/ relative to the execution path


Dependent on C Libraries:
========================

libevent http://libevent.org - efficient HTTP server

ImageMagick MagicWand API http://www.imagemagick.org/script/magick-wand.php - creating and transforming images in multiple formats


On OSX ImageMagick can be installed with homebrew:

    brew install imagemagick

On Debian/Ubuntu with apt-get:

    apt-get install libmagickwand-dev


Others can be installed from source: i.e. with: ./configure; make; make install


Hosting Viewers/HTML files
===========================

Place these under the imageCache directory, Nginx will serve all files from here



Nginx Conf
===========

An Nginx conf file is included which serves images directly from imageCache. If the image doesn't exist, it uses the server as a fallback (which generates the image).
