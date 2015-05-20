Image converter for raw dumps produced by so2-camera
====================================================

Converts and processes raw data from so2-camera to PNG images. Will
likely be merged into so2 camera.

Why PNG?
--------

- compressed format (unlike raw dumps, 1/3 smaller file size)
- lossless compression (unlike JPEG, WebP)
- simple format (unlike Tiff)
- widely supported (unlike WebP or Tiff)
- supports 16bit (unlike gif)

Headers
-------

PNG is a chunked file format and supports text chunks holding
standardized and custom content We employ standard tEXt chunks with
standard Keywords to ensure compatability (see make_png_header.c).

Dependencies
============
Depends on zLib and openCV.

Compilation
===========

From Linux run

````
cmake .
make
./imageconverter infile.raw outfile.png
````

or use the supplied shell script in `/images`.
