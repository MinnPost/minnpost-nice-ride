[Nice Ride MN](https://www.niceridemn.org/) data analysis.

## Data

Nice Ride MN released all of its 2011 data.  It is available at
[here](http://velotraffic.com/2012/01/nice-ride-mn-data-set-made-public/); 
included in this repository for convenience.

Unzip the data so that the contents are in the following folder:

```
data/NRMN_2011_reports/*
```

## Data processing

### Determine routes

Get [pgRouting](http://www.pgrouting.org/).

1. One way is to use [the kyngchaos binary for Mac](http://www.kyngchaos.com/software/postgres).
2. Installing from source on Mac should be something like 
    * get code from: http://www.pgrouting.org/download.html
    * ```brew install cmake```
    * ```cmake .```
    * ```make```
    
Install osm2pgrouting

brew install expat
git clone git://github.com/zzolo/osm2pgrouting-mac.git
make