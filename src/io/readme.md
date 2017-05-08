SO2 Camera: IO subsystem
========================

Converts and processes raw data from so2-camera to PNG images.

Why PNG?
--------

[PNG](http://www.libpng.org/pub/png/) is an ideal choice for scientific data, because

- compressed format (unlike raw dumps, 1/3 smaller file size)
- lossless compression (unlike JPEG, WebP)
- simple format (unlike Tiff)
- widely supported (unlike WebP or Tiff)
- supports 16bit (unlike gif)
- can usually be used for publishing (http://academia.stackexchange.com/questions/42624/is-the-png-file-format-acceptable-for-academic-papers)


Headers
-------

PNG is a chunked file format and supports text chunks holding
standardized and custom content We employ standard tEXt chunks with
standard Keywords to ensure compatability (see make_png_header.c).

PNG Text Chunks can be accessed with eg. imageMagick (identify -verbose)
Linux: pngcheck -t (Linux) or TweakPNG (Windows).


Dependencies
------------
Depends on [zLib](http://www.zlib.net/) and [openCV](http://opencv.org/) 3.0.

Depends on openCV (which will maybe replace by libpng in the future).
