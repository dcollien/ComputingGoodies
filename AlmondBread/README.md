AlmondBread Mandelbrot Server
-----------------------------

Listens on port 8081 by default. Serves images of the format:
    /X<some x value>_Y<some y value>_Z<some zoom value>.png

e.g.

    /X-0.5_Y0.0_Z8.png


Color Maps are located in ./colormaps/ relative to the execution path

Images are cached as files in ./imageCache/ relative to the execution path


Nginx Conf
===========

An Nginx conf file is included which serves images directly from imageCache. If the image doesn't exist, it uses the server as a fallback (which generates the image).
