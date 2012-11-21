"""
File to handle processing the Nice Ride MN data
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
input_year = sys.argv[1] if len(sys.argv) > 0 else None
years = [ '2011', '2012' ]
for y in years:

  ##########################################
  # 2011
  if (input_year == None) or (input_year != None and input_year == y):
  
    # First calculate how many.
    db.execute("SELECT * FROM stations_" + y)
    rows = db.fetchall()
    total = len(rows)
    
    pp('[%s] %s possible combinations. \n' % (y, total * (total - 1)))
    
    # First determine combinations.  We can have "duplicates" as one ways will
    # affect actual routes.
    combinations = {}
    matching = rows
    for row in rows:
      for match in matching:
        if '%s-%s' % (row[1], match[1]) in combinations:
          # Already in combinations
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
          
    # Truncate table
    pp("[%s] Truncate routes table. \n" % y)
    db.execute("TRUNCATE TABLE routes_" + y)
    committed = conn.commit()
      
    # Now go through each match, call routino, save entry in router.
    # Note that routino saves the output files in current directory.
    # super_bicycle is bike without one-way or turn restrictions.
    route_type = 'shortest'
    check_route_output = True
    routes_committed = 0
    routes_not_committed = 0
    transport_priority = ['bicycle', 'super_bicycle', 'foot', 'moped', 'motorbike', 'motorcar']
    r_path = os.path.join(path, 'routino-2.2/web')
    conf_path = os.path.join(path, 'routino-conf')
    # Assume all routes that start and stop at same station
    # go to the library downtown
    popular_location_lat = 44.98064594145076
    popular_location_lon = -93.27032089233398
    
    command = '%(path)s/bin/router --dir=%(path)s/data --profiles=%(c_path)s/profiles.xml  --translations=%(path)s/data/translations.xml --lat1=%(lat1)s --lon1=%(lon1)s --lat2=%(lat2)s --lon2=%(lon2)s --output-gpx-track --%(type)s --profile=%(transport)s --transport=bicycle'
    command_circle = '%(path)s/bin/router --dir=%(path)s/data --profiles=%(c_path)s/profiles.xml  --translations=%(path)s/data/translations.xml --lat1=%(lat1)s --lon1=%(lon1)s --lat2=%(lat2)s --lon2=%(lon2)s --lat3=%(lat3)s --lon3=%(lon3)s --output-gpx-track --%(type)s --profile=%(transport)s --transport=bicycle'
    out_gpx = '%s-track.gpx' % (route_type)
    
    for c, v in combinations.items():
      pp('[%s] Analyzing route: %s    ' % (y, c))
      
      # To handle the fact that some routes won't work with a given profile
      # we go through different transports
      transported = False;
      for transport in transport_priority:
        if transported == True:
          break
        
        # Create that special command, check for circle path
        if v['start'] == v['end']:
          pp('[Circle route]    ')
          route_this = command_circle % { 'path': r_path, 'c_path': conf_path, 'lat1': v['start_lat'], 'lon1': v['start_lon'], 'lat2': popular_location_lat, 'lon2': popular_location_lon, 'lat3': v['end_lat'], 'lon3': v['end_lon'], 'transport': transport, 'type': route_type }
        else:
          route_this = command % { 'path': r_path, 'c_path': conf_path, 'lat1': v['start_lat'], 'lon1': v['start_lon'], 'lat2': v['end_lat'], 'lon2': v['end_lon'], 'transport': transport, 'type': route_type }
          
        out = commands.getstatusoutput(route_this)
        
        # Check if route was alright, if not mark and try new one.
        if ('Routed OK' in out[1] and check_route_output == True) or check_route_output == False:
          pp('[Route found]    ')
          # Read data from pgx file
          gpx_dom = parse(out_gpx)
          points = gpx_dom.getElementsByTagName('trkpt')
          line_points = []
          for p in points:
            if p.hasAttribute('lat') and p.hasAttribute('lon'):
              line_points.append(ppygis.Point(float(p.getAttribute('lon')), float(p.getAttribute('lat')), srid=srid))
            else:
              pp('[missing lat/lon] ')
      
          if len(line_points) == 0:
            pp('[no points]    ')
            routes_not_committed += 1
          else:
            route = ppygis.LineString(line_points, srid=srid)
            # Put into DB
            db.execute("INSERT INTO routes_" + y + " (terminal_id_start, terminal_id_end, start_geom, end_geom, route_geom) VALUES (%s, %s, %s, %s, %s)",
              (v['start'], v['end'], ppygis.Point(float(v['start_lon']), float(v['start_lat']), srid=srid), ppygis.Point(float(v['end_lon']), float(v['end_lat']), srid=srid), route))
            committed = conn.commit()
            if committed == None:
              pp('[route committed for %s] ' % transport)
              routes_committed += 1
              transported = True
            else:
              pp('[route not committed] ')
              routes_not_committed += 1
        else:
          pp('[no route: %s] ' % transport)
          
      # Did we find a valid transport
      if transported == False:
        pp('[NOT TRANSPORTED] \n')
        pp('Attempted: \n %s \n' % route_this)
        break;
          
      # Remove gpx output file
      commands.getstatusoutput('rm %s' % out_gpx)
      pp('\n')
      
    # Some final stats
    pp('[%s] Routes committed: %s \n' % (y, routes_committed))
    pp('[%s] Routes not committed: %s \n' % (y, routes_not_committed))

# Close db connections
db.close()
conn.close()