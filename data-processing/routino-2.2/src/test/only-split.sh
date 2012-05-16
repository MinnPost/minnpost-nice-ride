#!/bin/sh

# Exit on error

set -e

# Test name

name=`basename $0 .sh`

# Slim or non-slim

if [ "$1" = "slim" ]; then
    slim="-slim"
    dir="slim"
else
    slim=""
    dir="fat"
fi

# Pruned or non-pruned

if [ "$2" = "prune" ]; then
    prune=""
    pruned="-pruned"
else
    prune="--prune-none"
    pruned=""
fi

# Create the output directory

dir="$dir$pruned"

[ -d $dir ] || mkdir $dir

# Run the programs under a run-time debugger

debugger=valgrind
debugger=

# Name related options

osm=$name.osm
log=$name$slim$pruned.log

option_prefix="--prefix=$name"
option_dir="--dir=$dir"

# Generic program options

option_planetsplitter="--loggable --tagging=../../xml/routino-tagging.xml --errorlog $prune"
option_filedumper="--dump-osm"

# Run planetsplitter

echo "Running planetsplitter"

echo ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm > $log
$debugger ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm >> $log

# Run filedumper

echo "Running filedumper"

echo ../filedumper$slim $option_dir $option_prefix $option_filedumper >> $log
$debugger ../filedumper$slim $option_dir $option_prefix $option_filedumper > $dir/$osm

