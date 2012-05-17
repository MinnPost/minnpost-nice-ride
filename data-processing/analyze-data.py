"""
File to handle processing the Nice Ride MN data
"""
import os
import psycopg2
import sys
from datetime import *
import ppygis
import commands

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

# First calculate how many.
db.execute("SELECT * FROM stations")
rows = db.fetchall()
combination_total = 0;
for i in range(len(rows), 0, -1):
  combination_total += i

pp('%s possible combinations \n' % (combination_total))

# Go through each combination.  First determine combinations.
combinations = {}
matching = rows
for row in rows:
  for match in matching:
    if ('%s-%s' % (row[1], match[1]) in combinations) or ('%s-%s' % (match[1], row[1]) in combinations):
      # found, so do nothing
      pp('')
    else:
      combinations['%s-%s' % (row[1], match[1])] = {
        'start': row[1],
        'end': match[1],
        'start_lat': row[4],
        'start_lon': row[5],
        'end_lat': match[4],
        'end_lon': match[5]
      }
  
# Now go through each match, call routino, save entry in router
r_path = os.path.join(path, 'routino-2.2/web')
command = '%(path)s/bin/router --dir=%(path)s/data --profiles=%(path)s/data/profiles.xml  --translations=%(path)s/data/translations.xml --lat1=%(lat1)s --lon1=%(lon1)s --lat2=%(lat2)s --lon2=%(lon2)s --output-gpx-track --shortest --transport=bicycle'
out_gpx = '%s/'
count = 0
for c, v in combinations.items():
  if count == 0:
    pp('Analyzing route: %s   ' % c)
    out = commands.getstatusoutput(command % { 'path': r_path, 'lat1': v['start_lat'], 'lon1': v['start_lon'], 'lat2': v['end_lat'], 'lon2': v['end_lon']})
    if 'Routed OK' in out[1]:
      pp('[Route found]')
    else:
      pp('[Route not found]')
      
    pp('\n')
  count += 1
  


# Close db connections
db.close()
conn.close()