#!/usr/bin/perl

# Command line

if($#ARGV<1 || $ARGV>2 || ! -f $ARGV[0])
  {
   die  "Usage: waypoints.pl <filename.osm> list\n".
        "       waypoints.pl <filename.osm> <name> <number>\n";
  }

# Parse the file

open(FILE,"<$ARGV[0]") || die "Cannot open '$ARGV[0]'\n";

%waypoints=();
@waypoints=();
@waypoint_lat=();
@waypoint_lon=();
$innode=0;

while(<FILE>)
  {
   if($innode)
     {
      if(m%<tag k='name' v='([^']+)'%)
        {
         push(@waypoints,$1);
         $waypoints{$1}=$#waypoints;
        }
      $innode=0 if(m%</node>%);
     }
   elsif(m%<node .* lat='([-.0-9]+)' *lon='([-.0-9]+)' *>%)
     {
      $innode=1;
      push(@waypoint_lat,$1);
      push(@waypoint_lon,$2);
     }
  }

close(FILE);

# Perform the action

if($ARGV[1] eq "list")
  {
   print join(" ",sort @waypoints)."\n";
   exit 0;
  }

if($waypoints{$ARGV[1]} ne "")
  {
   print "--lat$ARGV[2]=$waypoint_lat[$waypoints{$ARGV[1]}] --lon$ARGV[2]=$waypoint_lon[$waypoints{$ARGV[1]}]\n";
   exit 0;
  }

exit 1;
