"""
File to handle importing the Nice Ride MN data
into postgres tables.
"""
import csv
import os
import psycopg2
import sys
from datetime import *
import dateutil.parser


def ct(value):
  """
  Custom trim function for cleaning up strings
  """
  if value == "''" or value == '""' or value == '' or value == 'NULL':
    return None
  else:
    return value.strip()
  

def dt(value, return_type):
  """
  Custom date handler function for cleaning up strings
  """
  if value == 'NULL' or value == '':
    return None
  else:
    if return_type == 'date':
      return dateutil.parser.parse(value).date()
    else:
      return dateutil.parser.parse(value)
    

# Paths
path = os.path.dirname(__file__)

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Read locations file and insert into DB.  Clear DB first
sys.stdout.write("\nImporting to stations.")
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
          (ct(row[0]), ct(row[1]), ct(row[2]), float(row[3]), float(row[4]), dt(row[5], 'date')))
        committed = conn.commit()
        
  row_count += 1
  
sys.stdout.write("\nCommited %s rows to stations.\n" % (row_count))


# Read casual_subscriptions file and insert into DB.  Clear DB first
sys.stdout.write("\nImporting to casual_subscriptions.")
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
          (int(row[0]), float(row[1]), ct(row[2]), ct(row[3]), ct(row[4]), ct(row[5]), dt(row[6], ''), dt(row[7], ''), ct(row[8]), int(row[9])))
        committed = conn.commit()
        if row_count % 1000 == 0:
          sys.stdout.write('.')
        
  row_count += 1
  
sys.stdout.write("\nCommited %s rows to casual_subscriptions.\n" % (row_count))


# Read rentals file and insert into DB.  Clear DB first
sys.stdout.write("\nImporting to rentals.")
db.execute("TRUNCATE TABLE rentals")
committed = conn.commit()

locations_file = os.path.join(path, '../data/NRMN_2011_reports/NRMN_Rentals_2011_Season.csv')
reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
row_count = 0
for row in reader:
  if row_count > 0:
      if row[0]:
        # 16
        # rental_id character varying(16),
        # account_number character varying(32),
        # subscriber_id character varying(32),
        # start_date timestamp without time zone,
        # start_station character varying(128),
        # start_termninal character varying(16),
        # end_date timestamp without time zone,
        # end_station character varying(128),
        # end_termninal character varying(16),
        # duration_seconds numeric,
        # bike_id character varying(16),
        # gender character varying(8),
        # zip character varying(16),
        # subscription_type_a character varying(128),
        # subscription_type_b character varying(128),
        # birth_date date,
        db.execute("INSERT INTO rentals (rental_id, account_number, subscriber_id, start_date, start_station, start_termninal, end_date, end_station, end_termninal, duration_seconds, bike_id, gender, zip, subscription_type_a, subscription_type_b, birth_date) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
          (ct(row[0]), ct(row[1]), ct(row[2]), dt(row[3], ''), ct(row[4]), ct(row[5]), 
          dt(row[6], ''), ct(row[7]), ct(row[8]), float(row[9]), ct(row[10]), ct(row[11]),
          ct(row[12]), ct(row[13]), ct(row[14]), dt(row[15], 'date')))
        committed = conn.commit()
        if row_count % 1000 == 0:
          sys.stdout.write('.')
        
  row_count += 1
  
sys.stdout.write("\nCommited %s rows to rentals.\n" % (row_count))
  

# Close db connections
db.close()
conn.close()