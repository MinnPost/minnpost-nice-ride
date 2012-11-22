"""
File to produce some basic stats on data
"""
import os
import psycopg2
import sys
from datetime import *
import ppygis
import commands
from xml.dom.minidom import parse, parseString

def pp(value):
  """
  Wrapper for printing to the screen without a buffer.
  """
  sys.stdout.write(value)
  sys.stdout.flush()
    

# Vars
path = os.path.dirname(__file__)
srid = 4326

# Connect to database
conn = psycopg2.connect('dbname=minnpost_nice_ride user=postgres host=localhost')
db = conn.cursor()

# Years
years = [ '2011', '2012' ]
for y in years:
  # Count routes
  query = """
  SELECT COUNT(*) FROM (
    SELECT DISTINCT
      s.terminal_id, s.lat, s.lon,
      e.terminal_id, e.lat, e.lon
    FROM rentals_%s AS r
      INNER JOIN stations_%s AS s ON r.start_terminal = s.terminal_id
      INNER JOIN stations_%s AS e ON r.end_terminal = e.terminal_id
  ) AS routes
  """ % (y, y, y)
  db.execute(query)
  count = db.fetchone()
  print '[%s] Total routes taken: %s' % (y, count[0])
  
  # Count rentals
  query = """
  SELECT COUNT(*) FROM rentals_%s
  """ % (y)
  db.execute(query)
  count = db.fetchone()
  print '[%s] Total rentals/trips taken: %s' % (y, count[0])
  
  # Count stations
  query = """
  SELECT COUNT(*) FROM stations_%s
  """ % (y)
  db.execute(query)
  count = db.fetchone()
  print '[%s] Total stations taken: %s' % (y, count[0])
  
  
  
  