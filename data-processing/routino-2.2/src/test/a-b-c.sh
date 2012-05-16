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
option_router="--loggable --transport=motorcar --profiles=../../xml/routino-profiles.xml --translations=copyright.xml"

# Run planetsplitter

echo "Running planetsplitter"

echo ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm > $log
$debugger ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm >> $log

# Run filedumper

echo "Running filedumper"

echo ../filedumper$slim $option_dir $option_prefix $option_filedumper >> $log
$debugger ../filedumper$slim $option_dir $option_prefix $option_filedumper > $dir/$osm

# Waypoints

waypoints=`perl waypoints.pl $osm list`

# Run the router for each waypoint

for waypoint in $waypoints; do

    case $waypoint in
        *a) waypoint=`echo $waypoint | sed -e 's%a$%%'` ;;
        *) continue ;;
    esac

    echo "Running router : $waypoint"

    waypoint_a=`perl waypoints.pl $osm ${waypoint}a 1`
    waypoint_b=`perl waypoints.pl $osm ${waypoint}b 2`
    waypoint_c=`perl waypoints.pl $osm ${waypoint}c 3`

    [ -d $dir/$name-$waypoint ] || mkdir $dir/$name-$waypoint

    echo ../router$slim $option_dir $option_prefix $option_osm $option_router $waypoint_a $waypoint_b $waypoint_c >> $log
    $debugger ../router$slim $option_dir $option_prefix $option_osm $option_router $waypoint_a $waypoint_b $waypoint_c >> $log

    mv shortest* $dir/$name-$waypoint

    if [ "$pruned" = "" ]; then

        echo cmp $dir/$name-$waypoint/shortest-all.txt expected/$name-$waypoint.txt >> $log
        cmp $dir/$name-$waypoint/shortest-all.txt expected/$name-$waypoint.txt >> $log

    fi

done
