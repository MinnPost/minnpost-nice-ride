#!/bin/sh -x

version=2.11

# Download the file.

wget http://openlayers.org/download/OpenLayers-$version.tar.gz

# Uncompress it.

tar -xzf OpenLayers-$version.tar.gz

# Create a custom OpenLayers file

(cd OpenLayers-$version/build && python build.py ../../routino.cfg && cp OpenLayers.js ..)

# Copy the files.

cp -p  OpenLayers-$version/OpenLayers.js .
cp -pr OpenLayers-$version/img .
cp -pr OpenLayers-$version/theme .

# Delete the remainder

rm -rf OpenLayers-$version
