"""
File to produce JSON from a specific day of rides.
"""

import os
import psycopg2
import sys
from datetime import *
import time
import ppygis
import json

def pp(value):
  """
  Wrapper for printing to the screen without a buffer.
  """
  sys.stdout.write(value)
  sys.stdout.flush()
  
def write_json(obj, file_path, callback):
  """
  Function to handle outputing to JSON and JSONP files.
  """
  json_output = json.dumps(obj, sort_keys=True)

  # Output to JSON file
  output = open(file_path, 'w')
  output.write(json_output + "\n")
  output.close()
  # Output to JSONP file
  output = open(file_path + 'p', 'w')
  output.write('%s(%s)\n' % (callback, json_output))
  output.close()
    

# Paths
path = os.path.dirname(__file__)
srid = 4326

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Edit these values as need
export_date = date(2011, 5, 18)
start = datetime(2011, 5, 18, 4, 30)
end = datetime(2011, 5, 19, 4, 30)

# First get all the basic rentals in that period.
rentals_file = 'visualizations/data/rentals.json'
rentals_file = os.path.join(path, '../' + rentals_file)

db.execute("""
  SELECT rental_id, start_date, end_date, start_terminal, end_terminal, duration_seconds 
  FROM rentals 
  WHERE start_date <= %s AND end_date >= %s
  """, (end, start))
rentals = db.fetchall()
rentals_dict = {}

for r in rentals:
  nice_object = {
    's': r[1].isoformat(),
    'e': r[2].isoformat(),
    'st': r[3],
    'et': r[4],
    'd': str(r[5]),
  }
  rentals_dict[r[0]] = nice_object

# Write out
write_json(rentals_dict, rentals_file, 'callback_rentals')


# Then get each unique route.  There may be some
# duplicates so we use two different files to save
# some space.
routes_file = 'visualizations/data/routes.json'
routes_file = os.path.join(path, '../' + routes_file)

db.execute("""
  SELECT DISTINCT terminal_id_start || '-' || terminal_id_end AS route_id, 
    ST_AsGEoJSON(route_geom) AS route
  FROM rentals
    INNER JOIN routes
      ON rentals.start_terminal = routes.terminal_id_start
      AND rentals.end_terminal = routes.terminal_id_end
  WHERE start_date <= %s AND end_date >= %s
  """, (end, start))
routes = db.fetchall()
routes_dict = {}

for r in routes:
  nice_object = {
    'r': json.loads(r[1]),
  }
  routes_dict[r[0]] = nice_object

# Write out
write_json(routes_dict, routes_file, 'callback_routes')

# Create density graph data
d_avg_file = 'visualizations/data/density_average.json'
d_avg_file = os.path.join(path, '../' + d_avg_file)

db.execute("""
  SELECT * FROM average_day
  """, (end, start))
d_avg = db.fetchall()
d_avg_array = []

for r in d_avg:
  # Flot uses UTC timestamps (milliseconds) to deal with time
  # so we should use that.  First we add the time to our
  # export date.
  avg_timestamp = datetime.combine(export_date, r[1])
  avg_timestamp = time.mktime(avg_timestamp.timetuple()) * 1000
  d_avg_array.append([int(avg_timestamp), float(r[4])])

# Write out
write_json(d_avg_array, d_avg_file, 'callback_density_average')

# Close db connections
db.close()
conn.close()