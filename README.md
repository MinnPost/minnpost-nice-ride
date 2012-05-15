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

* [pgRouting](http://www.pgrouting.org/).
    1. Use [the kyngchaos binary for Mac](http://www.kyngchaos.com/software/postgres).
* Install [osm2pgrouting](http://www.pgrouting.org/docs/tools/osm2pgrouting.html).
    1. ```brew install expat```
    2. ```git clone git://github.com/zzolo/osm2pgrouting-mac.git```
    3. ```cd osm2pgrouting-mac; make```

### Get OSM Data

* Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    1. ```cd data; wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2```
    2. ```bunzip2 mpls-stpaul.osm.bz2```