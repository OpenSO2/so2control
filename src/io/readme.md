SO2 Camera: IO subsystem
========================

Dumps camera data to disk or converts and processes data to PNG images.

This implements the file output system, responsible for writing
the gathered data to disk and to process the data into reasonable
file formats. This could also do i.e. packaging files into a .tar
file.

Currently, there are three write modes:

- dumb mode, the camera data and all relevent headers are dumped to
  files
- png mode, the camera data is converted to pngs, and relevant
  headers are written to ancillary text chunks
- both

PNG mode is a lot slower, but produces smaller files which can be
easily viewed and used for further processing.
Dumb mode requires less processing and does not alter the original
data, but requires additional work for viewing and evalution.
Doing both maximizes the amount of work, but ensures data integrety (in
case either mode fails).

Why PNG?
--------

The PNG file format is good file format for scientific imagery. It
is wildly supported and the file format is comparatifly simple,
which makes integration relativly simple. Because of this, viewer
application exist for very nearly every platform. It supports image
compression, which can reduce file size by 1/3 to 2/3 compared to
raw dumps. Unlike JPEG [1], PNG compression is always lossless.
PNG nativly supports 16 (and more) bit image depth, unlike e.g. GIF.


[1] There is also a lossless JPEG compression, but that is not widely supported.

Headers
-------

PNG is a chunked file format and supports text chunks holding
standardized and custom content We employ standard tEXt chunks with
standard Keywords to ensure compatibility (see make_png_header.c).

Unfortunately, displaying text chunks is not widely supported in PNG
viewer applications, but specialized tools like [`pngcheck`](http://www.libpng.org/pub/png/apps/pngcheck.html)
and [`ImageMagick`](http://www.imagemagick.org) (`identify-verbose`)
can do this.

Dependencies
------------

Depends on openCV which can be installed on linux by running something like

````
$ # Debian, Ubuntu etc
$ sudo apt-get install libopencv-dev
$ # RHEL, Scientific Linux, CentOS, Fedora, etc
$ sudo yum install opencv-devel
````

Compilation
-----------

On Linux run

````
$ cmake .
$ make
$ ./io-cli infile.raw outfolder
````

Run Tests
---------

`````
$ cmake .
$ make
$ make test
`````
