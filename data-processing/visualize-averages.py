"""
File to handle processing data to create average day data.
We first find what the average day looks like as far as
number of bikes on the road for a given interval (probably
5 min).  Then we compare each day.
"""
import os
import psycopg2
import sys
from datetime import *
import ppygis

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

# Let's find the least different
db.execute("SELECT day, ABS(SUM(difference)) as total_diff, COUNT(id) AS total_count FROM average_all_days GROUP BY day HAVING COUNT(id) > 200 ORDER BY total_diff ASC")
result = db.fetchone()
pp("Least difference: is date: %s with difference of %s \n" % (result[0], result[1]))

# Date to visualize, use 'average' for average table
#v_date = date(2011, 9, 3)
#v_date = 'average'
v_date = result[0]


# Visualize
stagger = 3
counting = 0
pp("Visualizing %s: \n" % v_date)

if v_date == 'average':
  db.execute("SELECT * FROM average_day")
  average_days = db.fetchall()
  for r in average_days:
    if counting % stagger == 0:
      pp("%s - %s: " % (r[1], r[2]))
      for dot in range(int(r[4])):
        pp(".")
        
      pp("\n")
    counting += 1
else:
  total_diff = 0
  db.execute("SELECT * FROM average_all_days WHERE day = %s", (v_date,))
  average_days = db.fetchall()
  for r in average_days:
    if counting % stagger == 0:
      pp("%s - %s: " % (r[2], r[3]))
      for dot in range(int(r[4])):
        pp(".")
        
      pp("\n")
    counting += 1
    total_diff += r[5]
  
  pp("Total difference: %s \n" % total_diff)

pp("Visualized %s: \n" % v_date)

# Close db connections
db.close()
conn.close()