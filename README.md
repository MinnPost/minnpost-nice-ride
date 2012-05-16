[Nice Ride MN](https://www.niceridemn.org/) data analysis.

## Data

### Nice Ride data

Nice Ride MN released all of its 2011 data.  It is available at
[here](http://velotraffic.com/2012/01/nice-ride-mn-data-set-made-public/); 
included in this repository for convenience.

1. Unzip the data to work with:
    * ```cd data; unzip -d NRMN_2011_reports NRMN_2011_reports.zip;```

### Get OSM Data

1. Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    * ```cd data; wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2```
    * ```bunzip2 mpls-stpaul.osm.bz2```

## Data processing

### Prerequisites

1. Install [Routino](http://www.routino.org/).
    
### Calculate routes

./planetsplitter --tagging=../data/tagging-ride.xml --dir=../data ~/Code/mamp/minnpost/minnpost-nice-ride/data/mpls-stpaul.osm 

./router --dir=data --transport=bicycle --profiles=../data/profiles.xml  --translations=../data/translations.xml --shortest --output-text --lat1=44.989608 --lon1=-93.24206099999998 --lat2=44.939714 --lon2=-93.29049900000001

