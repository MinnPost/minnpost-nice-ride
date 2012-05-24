#!/usr/bin/env python
"""
Fab file to help with managing project.  For docs on Fab file, please see: http://docs.fabfile.org/

For exporting tiles to s3, copied form minnpost-base-maps
"""
import sys
import os
import warnings
import json
import re
from fabric.api import *

"""
Base configuration
"""
env.project_name = 'minnpost-nice-ride'
env.pg_host = 'localhost'
env.pg_dbname = 'minnpost_fec'
env.pg_user = 'postgres'
env.pg_pass = ''
env.labels = None
env.tile_scheme = 'xyz'
env.tile_template = '{z}/{x}/{y}'
env.bounds = None
env.bbox = None

# Tilemill paths.  For Ubuntu
if os.path.exists('/usr/share/tilemill'):
  env.tilemill_path = '/usr/share/tilemill'
  env.tilemill_projects = '/usr/share/mapbox/project'
  env.node_path = '/usr/bin/node'
  env.os = 'Ubuntu'
# OSX
else:
  env.tilemill_path = '/Applications/TileMill.app/Contents/Resources'
  env.tilemill_projects = '~/Documents/MapBox/project'
  env.node_path = '%(tilemill_path)s/node' % env
  env.os = 'OSX'
  
"""
Environments
"""
def production():
  """
  Work on production environment
  """
  env.settings = 'production'
  #env.s3_buckets = ['a.tiles.minnpost', 'b.tiles.minnpost', 'c.tiles.minnpost', 'd.tiles.minnpost']
  env.s3_buckets = ['a.tiles.minnpost']
  env.s3_template = 'http://{s}.tiles.minnpost.com'
  env.acl = 'acl-public'


def staging():
  """
  Work on staging environment
  """
  env.settings = 'staging'
  env.s3_buckets = ['testing.tiles.minnpost']
  env.s3_template = 'http://testing.tiles.minnpost.s3.amazonaws.com'
  env.acl = 'acl-public'
  

def map(name):
  """
  Select map to work on.
  """
  env.map = name
  

def tile_scheme(scheme='xyz'):
  """
  Change tilescheme.
  """
  env.tile_scheme = scheme
  

def bbox(b1=None, b2=None, b3=None, b4=None):
  """
  Change bounding box.
  """
  if b1 != None and b2 != None and b3 != None and b4 != None:
    env.bounds = [float(b1), float(b2), float(b3), float(b4)]
    env.bbox = '%s,%s,%s,%s' % (b1, b2, b3, b4)
  

def deploy_to_s3(concurrency):
  """
  Deploy tiles to S3.
  """
  require('settings', provided_by=[production, staging])
  require('map', provided_by=[map])
  env.concurrency = concurrency
  _create_map_suffix()

  # Deploy to many buckets (multi-dns-head mode)
  for bucket in env.s3_buckets:
    env.s3_bucket = bucket    
    local('ivs3 %(map)s/tiles %(s3_bucket)s/%(project_name)s/%(map)s%(map_suffix)s --%(acl)s -c %(concurrency)s' % env)


def export_deploy(concurrency=32, minzoom=None, maxzoom=None):
  """
  Deploy a map. Optionally takes a concurrency parameter indicating how many files to upload simultaneously.
  """
  require('settings', provided_by=[production, staging])
  require('map', provided_by=[map])
  
  create_exports()
  cleanup_exports()
  generate_mbtile(minzoom, maxzoom)
  generate_tiles_from_mbtile()
  generate_tilejson()
  deploy_to_s3(concurrency)
  reset_labels()
  

def link_caches():
  """
  Link local map cache to Mapbox cache to speed up mapnik conversion.
  """
  require('map', provided_by=[map])
  
  env.base_path = os.getcwd()
  exists = os.path.exists('%(base_path)s/%(map)s/cache' % env)
  if exists:
    print "There already a linked cache directory."
  else:
    env.tilemill_cache = os.path.expanduser('%(tilemill_projects)s/../cache/' % env)
    local(('ln -s %(tilemill_cache)s %(base_path)s/%(map)s/cache') % env)
  

def carto_to_mapnik():
  """
  Convert carto styles to mapnik configuration.  This should be able to
  be done with the Tilemill export API.
  """
  require('map', provided_by=[map])
  link_caches()
  local('%(tilemill_path)s/node_modules/carto/bin/carto %(map)s/project.mml > %(map)s/%(map)s.xml' % env)
  

def generate_mbtile(minzoom=None, maxzoom=None):
  """
  Generate MBtile.
  """
  require('map', provided_by=[map])
  
  # Read data from project mml
  with open('%(map)s/project.mml' % env, 'r') as f:
    config = json.load(f)
  
    # Define config values
    env.minzoom = config['minzoom'] if minzoom == None else minzoom
    env.maxzoom = config['maxzoom'] if maxzoom == None else maxzoom
    
    # If env bbox has not been set, then use config
    if env.bbox == None:
      env.bbox = '%f,%f,%f,%f' % (config['bounds'][0], config['bounds'][1], config['bounds'][2], config['bounds'][3])
    
    # Workaround for ICU
    if env.os == 'OSX':
      local('export ICU_DATA=%(tilemill_path)s/data/icu/' % env)
    
    # Export
    local('%(tilemill_path)s/node %(tilemill_path)s/index.js export --format=mbtiles --minzoom=%(minzoom)s --maxzoom=%(maxzoom)s --bbox=%(bbox)s %(map)s %(map)s/exports/%(map)s.mbtiles' % env)


def generate_tiles_from_mbtile():
  """
  Generate MBtile.
  """
  require('map', provided_by=[map])
  read_project()
    
  exists = os.path.exists('%(map)s/exports/%(map)s.mbtiles' % env)
  if exists:
    with settings(warn_only=True):
      local('rm -rf %(map)s/tiles-tmp' % env)
      local('rm -rf %(map)s/tiles' % env)
      local('mb-util --scheme=%(tile_scheme)s %(map)s/exports/%(map)s.mbtiles %(map)s/tiles-tmp' % env)
      local('mv "%(map)s/tiles-tmp/%(map_version)s/%(map_title)s" %(map)s/tiles' % env)
      local('mv %(map)s/tiles-tmp/metadata.json %(map)s/tiles/metadata.json' % env)
  else:
    print 'No MBTile file found in exports.'


def generate_tilejson():
  """
  Generate valid tilejson file.  This should be able to be accomplished
  with the api, such as http://localhost:20009/api/Project/:id
  """
  require('settings', provided_by=[production, staging])
  require('map', provided_by=[map])
  _create_map_suffix()
  
  # Utilize project data
  with open('%(map)s/project.mml' % env, 'r') as f:
    config = json.load(f)
    tilejson = {}
    
    # Base values
    tilejson['scheme'] = env.tile_scheme
    tilejson['tilejson'] = '2.0.0'
    tilejson['id'] = env.map
    
    # Attempt to get values from config
    try:
      tilejson['name'] = config['name'] if config.has_key('name') else ''
      tilejson['description'] = config['description'] if config.has_key('description') else ''
      tilejson['version'] = config['version'] if config.has_key('version') else '1.0.0'
      tilejson['attribution'] = config['attribution'] if config.has_key('attribution') else ''
      tilejson['legend'] = config['legend'] if config.has_key('legend') else ''
      tilejson['minzoom'] = config['minzoom'] if config.has_key('minzoom') else 0
      tilejson['maxzoom'] = config['maxzoom'] if config.has_key('maxzoom') else 22
      tilejson['bounds'] = config['bounds'] if config.has_key('bounds') else [-180, -90, 180, 90]
      tilejson['center'] = config['center'] if config.has_key('center') else null
    except KeyError:
      print 'Key error'
    
    # Template is in the metadata.json file
    with open('%(map)s/tiles/metadata.json' % env, 'r') as j:
      metadata = json.load(j)
      try:
        tilejson['template'] = metadata['template']
      except KeyError:
        tilejson['template'] = ''
    
    # Figure out template
    tilejson['grids'] = []
    tilejson['tiles'] = []
    for bucket in env.s3_buckets:
      env.s3_bucket = bucket 
      tilejson['grids'].append('http://%(s3_bucket)s.s3.amazonaws.com/%(project_name)s/%(map)s%(map_suffix)s/%(tile_template)s.grid.json' % env)
      tilejson['tiles'].append('http://%(s3_bucket)s.s3.amazonaws.com/%(project_name)s/%(map)s%(map_suffix)s/%(tile_template)s.png' % env)
    
    # Write regular and jsonp tilejson files
    tilejson_file = open('%(map)s/tiles/tilejson.json' % env, 'w')
    tilejson_file.write(json.dumps(tilejson, sort_keys = True, indent = 2))
    tilejson_file.close()
    tilejsonp_file = open('%(map)s/tiles/tilejson.jsonp' % env, 'w')
    tilejsonp_file.write('grid(%s)' % json.dumps(tilejson, sort_keys = True, indent = 2))
    tilejsonp_file.close()


def generate_tiles_mapnik(process_count, minzoom=None, maxzoom=None):
  """
  Render tile from mapnik configuration
  """
  env.process_count = process_count

  # Read data from project mml
  with open('%(map)s/project.mml' % env, 'r') as f:
    config = json.load(f)
  
    # Define config values
    env.minzoom = config['minzoom'] if minzoom == None else minzoom
    env.maxzoom = config['maxzoom'] if maxzoom == None else maxzoom
    env.minlon = config['bounds'][0]
    env.minlat = config['bounds'][1]
    env.maxlon = config['bounds'][2]
    env.maxlat = config['bounds'][3]

    # Cleanup tiles
    local('rm -rf %(map)s/tiles/*' % env)
    
    # Render tiles
    command = 'ivtile %(map)s/%(map)s.xml %(map)s/tiles %(maxlat)s %(minlon)s %(minlat)s %(maxlon)s %(minzoom)s %(maxzoom)s -p %(process_count)s'
    if 'buffer' in env:
      command += ' -b %(buffer)s'
    local(command % env)
  

def no_labels():
  """
  Updates tilemill project to no use labels
  """
  require('map', provided_by=[map])
  env.labels = False
  
  # Create a backup
  exists = os.path.exists('%(map)s/project.mml.orig' % env)
  if exists != True:
    local('cp %(map)s/project.mml %(map)s/project.mml.orig' % env);
  
  # From the original, load the json, find any label style
  # and remove.
  overwrite = open('%(map)s/project.mml' % env, 'w')
  with open('%(map)s/project.mml.orig' % env, 'r') as f:
    mml = json.load(f)
    
    # Process styles
    styles = []
    for index, item in enumerate(mml['Stylesheet']):
      if item != 'labels.mss':
        styles.append(item)
    mml['Stylesheet'] = styles
    
    overwrite.write(json.dumps(mml, sort_keys = True, indent = 2))
    overwrite.close()
  

def only_labels():
  """
  Updates tilemill project to use only labels
  """
  require('map', provided_by=[map])
  env.labels = True
  
  # Create a backup
  exists = os.path.exists('%(map)s/project.mml.orig' % env)
  if exists != True:
    local('cp %(map)s/project.mml %(map)s/project.mml.orig' % env);
  
  # From the original, load the json, keep palette and labels
  overwrite = open('%(map)s/project.mml' % env, 'w')
  with open('%(map)s/project.mml.orig' % env, 'r') as f:
    mml = json.load(f)
    
    # Process styles
    styles = []
    for index, item in enumerate(mml['Stylesheet']):
      if item == 'labels.mss' or item == 'palette.mss':
        styles.append(item)
    mml['Stylesheet'] = styles
    
    overwrite.write(json.dumps(mml, sort_keys = True, indent = 2))
    overwrite.close()
  

def reset_labels():
  """
  Resets any label changes process
  """
  require('map', provided_by=[map])
  
  # Check for orig
  exists = os.path.exists('%(map)s/project.mml.orig' % env)
  if exists:
    local('rm -f %(map)s/project.mml' % env);
    local('mv %(map)s/project.mml.orig %(map)s/project.mml' % env);
  else:
    print 'No label processing to reset.'
  

def _create_map_suffix():
  """
  Creates map suffix for deploying
  """
  if env.labels == None:
    env.map_suffix = ''
  if env.labels == True:
    env.map_suffix = '-labels'
  if env.labels == False:
    env.map_suffix = '-no-labels'


def read_project():
  """
  Get data from the TileMill project, to be used in other
  commands.
  """
  require('map', provided_by=[map])

  # Read JSON from file
  with open('%(map)s/project.mml' % env, 'r') as f:
    mml = json.load(f)
    env.map_title = mml['name']
    
    # Version is not always defined
    try:
      env.map_version = mml['version']
    except KeyError:
      env.map_version = '1.0.0'

def create_exports():
  """
  Create export directories
  """
  require('map', provided_by=[map])
  local('mkdir -p %(map)s/tiles' % env)
  local('mkdir -p %(map)s/exports' % env)
  

def cleanup_exports():
  """
  Cleanup export directories
  """
  require('map', provided_by=[map])
  local('rm -rf %(map)s/tiles-tmp' % env)
  local('rm -rf %(map)s/tiles/*' % env)
  local('rm -rf %(map)s/exports/*' % env)


def link():
  """
  Link a map into the MapBox directory
  """
  require('map', provided_by=[map])
  
  exists = os.path.exists(os.path.expanduser('%(tilemill_projects)s/%(map)s' % env))
  if exists:
    print "A directory with that name already exists in your MapBox projects."
  else:
    env.base_path = os.getcwd()
    local(('ln -s %(base_path)s/%(map)s/ %(tilemill_projects)s/%(map)s') % env)


def unlink():
  """
  unLink a map into the MapBox directory
  """
  require('map', provided_by=[map])
  
  exists = os.path.exists(os.path.expanduser('%(tilemill_projects)s/%(map)s' % env))
  if exists:
    local(('unlink %(tilemill_projects)s/%(map)s') % env)
  else:
    print "There is no directpry with that name in your MapBox projects."
  

def clone(name):
  """
  Clone a map to work on.
  """
  require('map', provided_by=[map])
  env.clone = name
  
  exists = os.path.exists('%(clone)s' % env)
  if exists:
    print "A directory with that name already exists in this project."
  else:
    local('cp -r %(map)s %(clone)s' % env)
    
    # Now link it
    env.map = env.clone
    link()


def remove_from_s3():
  """
  Remove map from S3
  """
  require('settings', provided_by=[production, staging])
  require('map', provided_by=[map])
  
  with settings(warn_only=True):
    for bucket in env.s3_buckets:
      env.s3_bucket = bucket 
      local('s3cmd del --recursive s3://%(s3_bucket)s/%(project_name)s/%(map)s' % env)