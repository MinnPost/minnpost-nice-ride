[Nice Ride MN](https://www.niceridemn.org/) data analysis.

## Data

### Nice Ride data

Nice Ride MN released all of its 2011 and 2012 data.  Included in this repository for convenience.

Note that the Subscriber data (date of birth, gender, etc) is removed from the original 2011 data at the request of [Anton Schieffer](http://antonschieffer.com/nice-ride-and-user-privacy-crossing-the-line/).


1. Unzip the data to work with:
    * ```cd data && unzip -d NRMN_2011_reports NRMN_2011_reports.zip; cd -;```
    * ```cd data && unzip -d NRMN-2012-usage NRMN-2012-usage.zip; cd -;```

### Get OSM Data

1. Get [OSM Download for Twin Cities](http://metro.teczno.com/#mpls-stpaul)
    * ```cd data && wget http://osm-metro-extracts.s3.amazonaws.com/mpls-stpaul.osm.bz2; cd -;```
    * ```cd data && bunzip2 mpls-stpaul.osm.bz2; cd -;```

## Data processing

### Prerequisites

1. Build [Routino](http://www.routino.org/).  We'll set things up in the web directory so that we can use the included web app to test our data.  The web app is not secure and has been modified to work on Mac.  The webapp should work in any directory that is hosted by Apache.
    * ```cd data-processing/routino-2.2 && make; cd -;```
    * Setup routing database: ```./data-processing/routino-2.2/web/bin/planetsplitter --tagging=data-processing/routino-2.2/web/data/tagging.xml --dir=data-processing/routino-2.2/web/data data/mpls-stpaul.osm```
    * (optional) For setting up web app for testing, install OpenLayers: ```cd data-processing/routino-2.2/web/www/openlayers && bash install.sh; cd -;```
    * (optional) Open ```data-processing/routino-2.2/web/www/routino/router.html``` in a browser to test some routes.  The database was made for bicycles so that is the only type of route that will work.
    
### Put Nice Ride Data in Postgres

In order to better work with the data, we will put the Nice Ride data into a PostGIS database.

1. Get Python requirements: ```pip install -r requirements.txt```
1. Create a new database from a PostGIS template: ```createdb -U postgres -h localhost -T template_postgis minnpost_nice_ride```
1. Create tables (note that this is destructive): ```psql -U postgres -h localhost -f data-processing/create-tables.sql minnpost_nice_ride```
1. Import data: ```python data-processing/import-nice-ride-data.py```
    
### Calculate routes
 
1. Once everything is in the database, we can create all possible routes with: ```python data-processing/create-routes.py```

### Averaging

1. Find the average of all days and the counts for each individual day: ```python data-processing/average-days.py```
    * This will take some serious time and energy.
1. Use the ```data-processing/visualize-days.py``` script to visualize days on the command line.

### Route tiles

There is a layer of routes we want to produce for our main visualization.

1. Link the Tilemill project to where Tilemill expects it.  This is really only needed for working on it in TileMill.  ```cd data-processing/tiles && fab map:minnpost-nice-ride-routes-2011 link; cd -;  cd data-processing/tiles && fab map:minnpost-nice-ride-routes-2012 link; cd -;```
1. To export to S3, run the following (change map name accordingly): ```cd data-processing/tiles && fab map:minnpost-nice-ride-routes-[2011|2012] production export_deploy:32,8,16; cd -;```


### Data for visualization

In order to use the data that in the main applicaiton, we convert things to JSON.  Run the following:

1. ```python data-processing/jsonify-day.py```

## Visualization

1. Work on visualization is separated out into different files, but needs to be embeddable for deployment.  Run the following to update the embeddable version: ```python visualizations/compile-embedded.py```