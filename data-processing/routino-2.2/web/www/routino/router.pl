#
# Routino generic router Perl script
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2011 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Use the directory paths script
require "paths.pl";

# Load the profiles variables
require "profiles.pl";

# Use the perl Time::HiRes module
use Time::HiRes qw(gettimeofday tv_interval);

$t0 = [gettimeofday];

# EDIT THIS to set the filename prefix for the routing database files.
$data_prefix="";

#
# Fill in the default parameters using the ones above (don't use executable compiled in defaults)
#

sub FillInDefaults
  {
   my(%params)=@_;

   $params{transport}=$routino->{transport} if(!defined $params{transport});

   my($transport)=$params{transport};

   foreach $highway (keys %{$routino->{highways}})
     {
      $key="highway-$highway";
      $value=$routino->{profile_highway}->{$highway}->{$transport};
      $params{$key}=$value if(!defined $params{$key});

      $key="speed-$highway";
      $value=$routino->{profile_speed}->{$highway}->{$transport};
      $params{$key}=$value if(!defined $params{$key});
     }

   foreach $property (keys %{$routino->{properties}})
     {
      $key="property-$property";
      $value=$routino->{profile_property}->{$property}->{$transport};
      $params{$key}=$value if(!defined $params{$key});
     }

   $params{oneway} =~ s/(true|on)/1/;
   $params{oneway} =~ s/(false|off)/0/;

   $params{turns} =~ s/(true|on)/1/;
   $params{turns} =~ s/(false|off)/0/;

   foreach $restriction (keys %{$routino->{restrictions}})
     {
      $key="$restriction";
      $value=$routino->{profile_restrictions}->{$restriction}->{$transport};
      $params{$key}=$value if(!defined $params{$key});
     }

   return %params;
  }


#
# Run the router
#

sub RunRouter
  {
   my($optimise,%params)=@_;

   # Combine all of the parameters together

   my($params)="--$optimise";

   foreach $key (keys %params)
     {
      $params.=" --$key=$params{$key}";
     }

   # Change directory

   mkdir $results_dir,0755 if(! -d $results_dir);
   chdir $results_dir;

   # Create a unique output directory

   chomp($uuid=`echo '$params' $$ | md5 | cut -f1 '-d '`);

   mkdir $uuid;
   chmod 0775, $uuid;
   chdir $uuid;

   # Run the router

   $params.=" --dir=$data_dir" if($data_dir);
   $params.=" --prefix=$data_prefix" if($data_prefix);
   $params.=" --loggable";

   system "$bin_dir/$router_exe $params > router.log 2>&1";

   (undef,undef,$cuser,$csystem) = times;
   $time=sprintf "time: %.3f CPU / %.3f elapsed",$cuser+$csystem,tv_interval($t0);

   $message="";

   if($? != 0)
     {
      $message=`tail -1 router.log`;
     }

   $result="";

   if(-f "$optimise.txt")
     {
      $result=`tail -1 $optimise.txt`;
      @result=split(/\t/,$result);
      $result = $result[4]." , ".$result[5];
     }

   # Return the results

   return($uuid,$time,$result,$message);
  }


#
# Return the output file
#

# Possible file formats

%suffixes=(
           "html"      => ".html",
           "gpx-route" => "-route.gpx",
           "gpx-track" => "-track.gpx",
           "text"      => ".txt",
           "text-all"  => "-all.txt",
           "log"       => ".log"
          );

# Possible MIME types

%mimetypes=(
            "html"      => "text/html",
            "gpx-route" => "text/xml",
            "gpx-track" => "text/xml",
            "text"      => "text/plain",
            "text-all"  => "text/plain",
            "log"       => "text/plain"
           );

sub ReturnOutput
  {
   my($uuid,$type,$format)=@_;

   if($type eq "router") { $format="log" }

   $suffix=$suffixes{$format};
   $mime  =$mimetypes{$format};

   $file="$results_dir/$uuid/$type$suffix";

   # Return the output

   if(!$type || !$uuid || !$format || ! -f $file)
     {
      print header('text/plain','404 Not found');
      print "Not Found!\n";
     }
   else
     {
      print header($mime);

      system "cat $file";
     }
  }

1;
