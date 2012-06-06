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

# Clear out average tabled
pp("Truncate average_day table. \n")
db.execute("TRUNCATE TABLE average_day")
committed = conn.commit()
pp("Truncate average_all_days table. \n")
db.execute("TRUNCATE TABLE average_all_days")
committed = conn.commit()

# Let's get the total number of rides
db.execute("SELECT COUNT(id) AS total_ride FROM rentals")
ride_count = db.fetchone()[0]
pp("Number of rides in season: %s \n" % ride_count)

# Let's get the maximum and minimum dates as bikes are only
# available for part of the year
db.execute("SELECT MAX(end_date) AS max_date FROM rentals")
max_date = db.fetchone()[0].date()
db.execute("SELECT MIN(start_date) AS max_date FROM rentals")
min_date = db.fetchone()[0].date()
season_days = (max_date - min_date).days
pp("Number of days in season: %s \n" % season_days)

# Create rows for intervals, update seconds to change interval
seconds = 5 * 60
interval = timedelta(seconds = seconds)
start_time = time().min
end_time = time().max
times = (24 * 60 * 60) / seconds
count = 0

pp("Adding rows at interval: %s " % interval)

# Go through each interval.  Note that time interval does
# not work for times, just datetimes
interval_end = start_time
while count < times:
  interval_start = interval_end
  interval_end = (datetime.combine(date.today(), interval_start) + interval).time()
  
  # Determine counts and average.  Tkae into account rides that go over midnight,
  # assuming less than 24 hour rides
  db.execute("""
    SELECT COUNT(id) AS interval_count FROM rentals 
    WHERE ((
      CAST(start_date AS time) < CAST(end_date AS time)
      AND CAST(start_date AS time) < %s 
      AND CAST(end_date AS time) > %s
    ) 
    OR (
      CAST(start_date AS time) > CAST(end_date AS time)
      AND CAST(start_date AS time) > %s 
      AND CAST(end_date AS time) > %s
    ))
    AND CAST(start_date AS time) <> CAST(end_date AS time)
    AND start_date IS NOT NULL
    AND end_date IS NOT NULL
  """, (interval_end, interval_start, interval_end, interval_start))
  rides = db.fetchone()[0]
  
  # Save
  db.execute("INSERT INTO average_day (start_time, end_time, total, average) VALUES (%s, %s, %s, %s)", (interval_start, interval_end, rides, float(float(rides) / float(season_days))))
  committed = conn.commit()
  pp(".")
  count += 1

pp("\n")
  
# Figure out what day is "most average"
pp("Compare all days to find the most average. \n")

db.execute("SELECT * FROM average_day")
average_days = db.fetchall()
diffs = {}
current_date = min_date

while current_date <= max_date:
  pp("Compare %s: " % current_date)
  total_diff = 0;
  
  for avg in average_days:
    # Determine counts and average.  (try) Take into account rides that go over midnight
    db.execute("""
      SELECT COUNT(id) AS interval_count FROM rentals 
      WHERE ((
        CAST(start_date AS time) < CAST(end_date AS time)
        AND CAST(start_date AS time) < %s 
        AND CAST(end_date AS time) > %s
      ) 
      OR (
        CAST(start_date AS time) > CAST(end_date AS time)
        AND CAST(start_date AS time) < %s 
        AND CAST(end_date AS time) < %s
      ))
      AND CAST(start_date AS time) <> CAST(end_date AS time)
      AND start_date IS NOT NULL
      AND end_date IS NOT NULL
      AND (
        CAST(start_date AS date) = %s
        OR CAST(start_date AS date) = %s
      )
    """, (avg[2], avg[1], avg[2], avg[1], current_date, current_date))
    rides = db.fetchone()[0]
    
    # Calculate difference as absolute difference of bikes.  This is the
    # total area of difference
    interval_diff = abs(float(rides) - float(avg[4]))
  
    # Save into all days table
    db.execute("INSERT INTO average_all_days (day, start_time, end_time, total, difference) VALUES (%s, %s, %s, %s, %s)", (current_date, avg[1], avg[2], rides, interval_diff))
    committed = conn.commit()
    total_diff += interval_diff
  
  # Output difference and increment day
  pp("%s difference \n" % total_diff)
  current_date += timedelta(days = 1)

# Close db connections
db.close()
conn.close()