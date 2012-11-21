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
import ppygis


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


def pp(value):
  """
  Wrapper for printing to the screen without a buffer.
  """
  sys.stdout.write(value)
  sys.stdout.flush()


# Paths
path = os.path.dirname(__file__)

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Years
input_year = sys.argv[1] if len(sys.argv) > 0 else None
years = [ '2011', '2012' ]
for y in years:

  ##########################################
  # 2011
  if (input_year == None and y == '2011') or (input_year != None and input_year == '2011' and y == '2011'):
    # Read locations file and insert into DB.  Clear DB first
    pp("\n[%s] Importing to stations." % y)
    db.execute("TRUNCATE TABLE stations_" + y)
    committed = conn.commit()
            
    locations_file = os.path.join(path, '../data/2011_Station_Locations.csv')
    reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
    row_count = 0
    for row in reader:
      if row_count > 0:
          if row[0]:
            # terminal_id character varying(16), common_name character varying(128), 
            # station character varying(128), lat numeric, lon numeric, install_date date,
            db.execute("INSERT INTO stations_" + y + " (terminal_id, common_name, station, lat, lon, install_date, location_geom) VALUES (%s, %s, %s, %s, %s, %s, %s)" ,
              (ct(row[0]), ct(row[1]), ct(row[2]), float(row[3]), float(row[4]), dt(row[5], 'date'), ppygis.Point(float(row[4]), float(row[3]), srid=4326)))
            committed = conn.commit()
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to stations.\n" % (y, row_count))
    
    
    # Read casual_subscriptions file and insert into DB.  Clear DB first
    pp("\n[%s] Importing to casual_subscriptions." % y)
    db.execute("TRUNCATE TABLE casual_subscriptions_" + y)
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
            db.execute("INSERT INTO casual_subscriptions_" + y + " (number_rentals, duration_minutes, subscriber_id, subscription_type, zip, status, activation_start_date, activation_end_date, gc_friendly_name, bikes) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
              (int(row[0]), float(row[1]), ct(row[2]), ct(row[3]), ct(row[4]), ct(row[5]), dt(row[6], ''), dt(row[7], ''), ct(row[8]), int(row[9])))
            committed = conn.commit()
            if row_count % 1000 == 0:
              pp('.')
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to casual_subscriptions.\n" % (y, row_count))
    
    
    # Read rentals file and insert into DB.  Clear DB first
    pp("\n[%s] Importing to rentals." % y)
    db.execute("TRUNCATE TABLE rentals_" + y)
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
            # start_terminal character varying(16),
            # end_date timestamp without time zone,
            # end_station character varying(128),
            # end_terminal character varying(16),
            # duration_seconds numeric,
            # bike_id character varying(16),
            # gender character varying(8),
            # zip character varying(16),
            # subscription_type_a character varying(128),
            # subscription_type_b character varying(128),
            # birth_date date,
            db.execute("INSERT INTO rentals_" + y + " (rental_id, account_number, subscriber_id, start_date, start_station, start_terminal, end_date, end_station, end_terminal, duration_seconds, bike_id, gender, zip, subscription_type_a, subscription_type_b, birth_date) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
              (ct(row[0]), ct(row[1]), ct(row[2]), dt(row[3], ''), ct(row[4]), ct(row[5]), 
              dt(row[6], ''), ct(row[7]), ct(row[8]), float(row[9]), ct(row[10]), ct(row[11]),
              ct(row[12]), ct(row[13]), ct(row[14]), dt(row[15], 'date')))
            committed = conn.commit()
            if row_count % 1000 == 0:
              pp('.')
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to rentals.\n" % (y, row_count))
    
    
    # Read subscribers file and insert into DB.  Clear DB first
    pp("\n[%s] Importing to subscribers." % y)
    db.execute("TRUNCATE TABLE subscribers_" + y)
    committed = conn.commit()
    
    locations_file = os.path.join(path, '../data/NRMN_2011_reports/Subscribers_2011_Season.csv')
    reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
    row_count = 0
    for row in reader:
      if row_count > 0:
          if row[0]:
            # 14
            # gender character varying(8),
            # number_rentals integer,
            # duration_minutes numeric,
            # account_number character varying(32),
            # subscriber_id character varying(32),
            # subscription_type character varying(128),
            # activation_status character varying(16),
            # city character varying(128),
            # state character varying(32),
            # zip character varying(16),
            # status character varying(16),
            # birth_date date,
            # start_date timestamp without time zone,
            # end_date timestamp without time zone,
            db.execute("INSERT INTO subscribers_" + y + " (gender, number_rentals, duration_minutes, account_number, subscriber_id, subscription_type, activation_status, city, state, zip, status, birth_date, start_date, end_date) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
              (ct(row[0]), int(row[1]), float(row[2]), ct(row[3]), ct(row[4]), ct(row[5]), 
              ct(row[6]), ct(row[7]), ct(row[8]), ct(row[9]), ct(row[10]), dt(row[11], 'date'),
              dt(row[12], ''), dt(row[13], '')))
            committed = conn.commit()
            if row_count % 1000 == 0:
              pp('.')
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to subscribers.\n" % (y, row_count))
    
  ##########################################
  # 2012
  elif (input_year == None and y == '2012') or (input_year != None and input_year == '2012' and y == '2012'):
    pp("\n[%s] Importing to stations." % y)
    db.execute("TRUNCATE TABLE stations_" + y)
    committed = conn.commit()
            
    locations_file = os.path.join(path, '../data/NRMN-2012-usage/2012-station_locations .csv')
    reader = csv.reader(open(locations_file, 'rU'), delimiter=',', dialect=csv.excel_tab)
    row_count = 0
    for row in reader:
      if row_count > 0:
          if row[0]:
            # terminal_id character varying(16), common_name character varying(128), 
            # station character varying(128), lat numeric, lon numeric, install_date date,
            db.execute("INSERT INTO stations_" + y + " (terminal_id, station, lat, lon, bike_docks, location_geom) VALUES (%s, %s, %s, %s, %s, %s)" ,
              (ct(row[0]), ct(row[1]), float(row[2]), float(row[3]), int(row[4]), ppygis.Point(float(row[4]), float(row[3]), srid=4326)))
            committed = conn.commit()
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to stations.\n" % (y, row_count))
    
    
    # Read rentals file and insert into DB.  Clear DB first
    pp("\n[%s] Importing to rentals." % y)
    db.execute("TRUNCATE TABLE rentals_" + y)
    committed = conn.commit()
    
    locations_file = os.path.join(path, '../data/NRMN-2012-usage/2012-all-rentals.csv')
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
            # start_terminal character varying(16),
            # end_date timestamp without time zone,
            # end_station character varying(128),
            # end_terminal character varying(16),
            # duration_seconds numeric,
            # bike_id character varying(16),
            # gender character varying(8),
            # zip character varying(16),
            # subscription_type_a character varying(128),
            # subscription_type_b character varying(128),
            # birth_date date,
            db.execute("INSERT INTO rentals_" + y + " (rental_id, subscriber_id, start_date, start_station, start_terminal, end_date, end_station, end_terminal, duration_seconds, bike_id, zip, subscription_type_a) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
              (ct(row[0]), ct(row[1]), dt(row[2], ''), ct(row[3]), ct(row[4]), 
              dt(row[5], ''), ct(row[6]), ct(row[7]), float(row[8]), ct(row[9]), ct(row[10]),
              ct(row[11])))
            committed = conn.commit()
            if row_count % 1000 == 0:
              pp('.')
            
      row_count += 1
      
    pp("\n[%s] Commited %s rows to rentals.\n" % (y, row_count))
  

# Close db connections
db.close()
conn.close()
  

# Close db connections
db.close()
conn.close()