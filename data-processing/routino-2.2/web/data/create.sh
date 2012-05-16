#!/bin/sh -x

# This script can download either from GeoFabrik or Cloudmade.


# EDIT THIS to set the names of the files to download.
files="europe/great_britain.osm.bz2 europe/ireland.osm.bz2 europe/isle_of_man.osm.bz2"

# Download the files

for file in $files; do
   wget -N http://download.geofabrik.de/osm/$file
done


## EDIT THIS to set the names of the files to download.
#files="europe/united_kingdom/united_kingdom.osm.bz2 europe/ireland/ireland.osm.bz2 europe/isle_of_man/isle_of_man.osm.bz2"
#
## Download the files
#
#for file in $files; do
#   wget -N http://downloads.cloudmade.com/$file
#done


# Process the data

bunzip2 *.bz2

../bin/planetsplitter --errorlog *.osm
