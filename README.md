[Nice Ride MN](https://www.niceridemn.org/) data analysis.

## Data

Nice Ride MN released all of its 2011 data.  It is available at
[here](http://velotraffic.com/2012/01/nice-ride-mn-data-set-made-public/); 
included in this repository for convenience.

Unzip the data to work with:

```
cd data; unzip -d NRMN_2011_reports NRMN_2011_reports.zip;
```

## Data processing

### Prerequisites

* Instal GAUL with custom homebrew formula as taken from [charlax fork of homebrew](https://github.com/charlax/homebrew/blob/gaul/Library/Formula/gaul.rb).
    1. ```cp data-processing/gaul/gaul.rb /usr/local/Library/Formula/gaul.rb```
    1. ```brew install gaul```
    1. Do note that this will much up your homebrew a bit.
* [pgRouting](http://www.pgrouting.org/).
    1. Install from source
    1. ```brew install cmake```
    1. ```brew install boost```
    1. ```brew install cgal```
    1. ```cp data-processing/pgrouting/pgrouting.rb /usr/local/Library/Formula/pgrouting.rb```
    1. ```brew install pgrouting```
* Install [osm2pgrouting](http://www.pgrouting.org/docs/tools/osm2pgrouting.html).  We are using a [custom Mac version](git clone git://github.com/zzolo/osm2pgrouting-mac.git).
    1. ```brew install expat```
    1. If you had not done this for pgrouting: ```git submodule init; git submodule update;```
    1. ```cd data-processing/osm2pgrouting-mac; cmake .```

### Get OSM Data

* Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    1. ```cd data; wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2```
    1. ```bunzip2 mpls-stpaul.osm.bz2```
* Create new PostGIS database: ```createdb -T template_postgis -U postgres -h localhost minnpost_nice_ride_routing;```
* Put OSM data into a POSTGIS database.
    1. ```./data-processing/osm2pgrouting-mac/osm2pgrouting -file data/mpls-stpaul.osm -conf data-processing/osm2pgrouting-mac/mapconfig.xml -dbname minnpost_nice_ride_routing -user postgres -host localhost -clean;```
    
### Calculate routes


