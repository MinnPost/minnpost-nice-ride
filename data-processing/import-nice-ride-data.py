"""
File to handle importing the Nice Ride MN data
into postgres tables.
"""
import csv
import os
import psycopg2
from datetime import *
import dateutil.parser


def ct(value):
  """
  Custom trim function for cleaning up strings
  """
  if value == "''" or value == '""':
    value = ''
  return value.strip()
    

# Paths
path = os.path.dirname(__file__)

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Read locations file and insert into DB.  Clear DB first
db.execute("TRUNCATE TABLE stations")
committed = conn.commit()
        
locations_file = os.path.join(path, '../data/2011_Station_Locations.csv')
reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
row_count = 0
for row in reader:
  if row_count > 0:
      if row[0]:
        # terminal_id character varying(16), common_name character varying(128), 
        # station character varying(128), lat numeric, lon numeric, install_date date,
        db.execute("INSERT INTO stations (terminal_id, common_name, station, lat, lon, install_date) VALUES (%s, %s, %s, %s, %s, %s)",
          (ct(row[0]), ct(row[1]), ct(row[2]), float(row[3]), float(row[4]), dateutil.parser.parse(row[5]).date()))
        committed = conn.commit()
        
  row_count += 1
  
db.execute("VACCUUM ANALYZE stations")
committed = conn.commit()
print "Commit %s rows to stations" % (row_count)


# Read casual_subscriptions file and insert into DB.  Clear DB first
db.execute("TRUNCATE TABLE casual_subscriptions")
committed = conn.commit()

locations_file = os.path.join(path, '../data/NRMN_2011_reports/Casual_subscriptions_2011_Season.csv')
reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
row_count = 0
for row in reader:
  if row_count > 0:
      if row[0]:
        # 10
        # number_rentals integer, duration_minutes numeric, subscriber_id character varying(32), 
        # subscription_type character varying(128), zip character varying(16), status character varying(16), 
        # activation_start_date timestamp without time zone, activation_end_date timestamp without time zone, 
        # gc_friendly_name character varying(128), bikes integer,
        db.execute("INSERT INTO casual_subscriptions (number_rentals, duration_minutes, subscriber_id, subscription_type, zip, status, activation_start_date, activation_end_date, gc_friendly_name, bikes) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
          (int(row[0]), float(row[1]), ct(row[2]), ct(row[3]), ct(row[4]), ct(row[5]), dateutil.parser.parse(row[6]), dateutil.parser.parse(row[7]), ct(row[8]), int(row[9])))
        committed = conn.commit()
        
  row_count += 1
  
db.execute("VACCUUM ANALYZE casual_subscriptions")
committed = conn.commit()
print "Commit %s rows to casual_subscriptions" % (row_count)
  
# Close db connections
db.close()
conn.close()