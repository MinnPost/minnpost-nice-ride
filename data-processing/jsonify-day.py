"""
File to produce JSON from a specific day of rides.
"""

import os
import psycopg2
import sys
from datetime import *
import ppygis
import json

def pp(value):
  """
  Wrapper for printing to the screen without a buffer.
  """
  sys.stdout.write(value)
  sys.stdout.flush()
    

# Paths
path = os.path.dirname(__file__)
srid = 4326

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Edit these values as need
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

# Create JSON to write
rentals_json = json.dumps(rentals_dict, sort_keys=True, indent=2)

# Output to JSON file
rentals_output = open(rentals_file, 'w')
rentals_output.write(rentals_json + "\n")
rentals_output.close()
# Output to JSONP file
rentals_output = open(rentals_file + 'p', 'w')
rentals_output.write('callback_rentals(' + rentals_json + ")\n")
rentals_output.close()


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
    'r': r[1],
  }
  routes_dict[r[0]] = nice_object

# Create JSON to write
routes_json = json.dumps(routes_dict, sort_keys=True, indent=2)

# Output to JSON file
routes_output = open(routes_file, 'w')
routes_output.write(routes_json + "\n")
routes_output.close()
# Output to JSONP file
routes_output = open(routes_file + 'p', 'w')
routes_output.write('callback_routes(' + routes_json + ")\n")
routes_output.close()

# Close db connections
db.close()
conn.close()