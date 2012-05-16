[Nice Ride MN](https://www.niceridemn.org/) data analysis.

## Data

### Nice Ride data

Nice Ride MN released all of its 2011 data.  It is available at
[here](http://velotraffic.com/2012/01/nice-ride-mn-data-set-made-public/); 
included in this repository for convenience.

1. Unzip the data to work with:
    * ```cd data && unzip -d NRMN_2011_reports NRMN_2011_reports.zip; cd -;```

### Get OSM Data

1. Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    * ```cd data && wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2; cd -;```
    * ```cd data && bunzip2 mpls-stpaul.osm.bz2; cd -;```

## Data processing

### Prerequisites

1. Build [Routino](http://www.routino.org/).  We'll set things up in the web directory so that we can use the included web app to test our data.  The web app is not secure and has been modified to work on Mac.  The webapp should work in any directory that is hosted by Apache.
    * ```cd data-processing/routino-2.2 && make; cd -;```
    * Setup routing database: ```./data-processing/routino-2.2/web/bin/planetsplitter --tagging=data-processing/routino-2.2/web/data/tagging-ride.xml --dir=data-processing/routino-2.2/web/data data/mpls-stpaul.osm```
    * (optional) For setting up web app for testing, install OpenLayers: ```cd data-processing/routino-2.2/web/www/openlayers && bash install.sh; cd -;```
    * (optional) Open ```data-processing/routino-2.2/web/www/routino/router.html``` in a browser to test some routes.  The database was made for bicycles so that is the only type of route that will work.
    
### Put Nice Ride Data in Postgres

In order to better work with the data, we will put the Nice Ride data into a PostGIS database.

1. Get Python requirements: ```pip install -r requirements.txt```
1. Create a new database from a PostGIS template: ```createdb -U postgres -h localhost -T template_postgis minnpost_nice_ride```
1. Create tables (not that this is destructive): ```psql -U postgres -h localhost -f data-processing/create-tables.sql minnpost_nice_ride```
1. Import data: ```python data-processing/import-nice-ride-data.py```
    
### Calculate routes
 

./router --dir=data --transport=bicycle --profiles=../data/profiles.xml  --translations=../data/translations.xml --shortest --output-text --lat1=44.989608 --lon1=-93.24206099999998 --lat2=44.939714 --lon2=-93.29049900000001

