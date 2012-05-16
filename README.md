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

1. Instal GAUL with custom homebrew formula as taken from [charlax fork of homebrew](https://github.com/charlax/homebrew/blob/gaul/Library/Formula/gaul.rb).
    * ```cp data-processing/gaul/gaul.rb /usr/local/Library/Formula/gaul.rb```
    * ```brew install gaul```
    * Do note that this will much up your homebrew a bit.
1. Install [pgRouting](http://www.pgrouting.org/).  Unfortunately there is no easy way to do this with homebrew.
    * Download and install from [KyngChaos](http://www.kyngchaos.com/software/postgres).
    * Assuming you have a template_postgis, create a template for routing: ```createdb -U postgres -h localhost -T template_postgis template_routing```
    * Install pgrouting functions to template: ```psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_core.sql template_routing; psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_core_wrappers.sql template_routing; psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_tsp.sql template_routing; psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_tsp_wrappers.sql template_routing; psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_dd.sql template_routing; psql -U postgres -h localhost -f /usr/local/pgsql-9.1/share/contrib/routing_dd_wrappers.sql template_routing;```
1. Install [osm2pgrouting](http://www.pgrouting.org/docs/tools/osm2pgrouting.html).  We are using a [custom Mac version](git clone git://github.com/zzolo/osm2pgrouting-mac.git).
    * ```brew install expat```
    * If you had not done this for pgrouting: ```git submodule init; git submodule update;```
    * ```cd data-processing/osm2pgrouting-mac; cmake .```

### Get OSM Data

1. Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    * ```cd data; wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2```
    * ```bunzip2 mpls-stpaul.osm.bz2```
1. Create new PostGIS database: ```createdb -T template_routing -U postgres -h localhost minnpost_nice_ride_routing;```
1. Put OSM data into a POSTGIS database.
    * ```./data-processing/osm2pgrouting-mac/osm2pgrouting -file data/mpls-stpaul.osm -conf data-processing/osm2pgrouting-mac/mapconfig.xml -dbname minnpost_nice_ride_routing -user postgres -host localhost -clean;```
    
### Calculate routes


