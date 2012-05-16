#!/usr/bin/perl
#
# Routino non-Javascript router CGI
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

# Use the generic router script
require "router.pl";

# Use the perl CGI module
use CGI ':cgi';

# Create the query and get the parameters

$query=new CGI;

@rawparams=$query->param;

# Legal CGI parameters with regexp validity check

%legalparams=(
              "lon[1-9]"        => "[-0-9.]+",
              "lat[1-9]"        => "[-0-9.]+",
              "transport"       => "[a-z]+",
              "highway-[a-z]+"  => "[0-9.]+",
              "speed-[a-z]+"    => "[0-9.]+",
              "property-[a-z]+" => "[0-9.]+",
              "oneway"          => "(1|0|true|false|on|off)",
              "turns"           => "(1|0|true|false|on|off)",
              "weight"          => "[0-9.]+",
              "height"          => "[0-9.]+",
              "width"           => "[0-9.]+",
              "length"          => "[0-9.]+",
              "length"          => "[0-9.]+",

              "language"        => "[-a-zA-Z]+",
              "submit"          => "(shortest|quickest|link)",
              "format"          => "(html|gpx-route|gpx-track|text|text-all|form)"
             );

# Validate the CGI parameters, ignore invalid ones

foreach $key (@rawparams)
  {
   foreach $test (keys (%legalparams))
     {
      if($key =~ m%^$test$%)
        {
         $value=$query->param($key);

         if($value =~ m%^$legalparams{$test}$%)
           {
            $cgiparams{$key}=$value;
            last;
           }
        }
     }
  }

# Get the important parameters

$submit=$cgiparams{submit};
delete $cgiparams{submit};

$format=$cgiparams{format};
delete $cgiparams{format};

$format="form" if(!$format || ($submit ne "shortest" && $submit ne "quickest"));

# Generate a custom URL

$customurl="";

if($submit)
  {
   $customurl="noscript.cgi?";

   foreach $key (sort (keys %cgiparams))
     {
      $customurl.="$key=$cgiparams{$key};";
     }

   $customurl.="submit=custom";
  }

# Fill in the default parameters

%fullparams=FillInDefaults(%cgiparams);

# Open template file before running the router (including changing directory)

open(TEMPLATE,"<noscript.template.html");

# Run the router

if($submit eq "shortest" || $submit eq "quickest")
  {
   ($router_uuid,$router_time,$router_result,$router_message)=RunRouter($submit,%fullparams);

   $router_type=$submit;
   $router_Type="Shortest" if($submit eq "shortest");
   $router_Type="Quickest" if($submit eq "quickest");

   if($format ne "form")
     {
      ReturnOutput($router_uuid,$submit,$format);
      exit;
     }
  }

# Generate the form output

print header('text/html');

# Parse the template and fill in the parameters

while(<TEMPLATE>)
  {
   if (m%<input% && m%<!-- ([^ ]+) *-->%)
     {
      $key=$1;

      m%type="([a-z]+)"%;
      $type=$1;

      m%value="([a-z]+)"%;
      $value=$1;

      if ($type eq "radio")
        {
         $checked="";
         $checked="checked" if($fullparams{$key} eq $value);

         s%><!-- .+? *-->% $checked>%;
        }
      elsif ($type eq "checkbox")
        {
         $checked="";
         $checked="checked" if($fullparams{$key});

         s%><!-- .+? *-->% $checked>%;
        }
      elsif ($type eq "text")
        {
         s%><!-- .+? *-->% value="$fullparams{$key}">%;
        }

      print;
     }
   elsif (m%<!-- custom-url -->%)
     {
      s%<!-- custom-url -->%$customurl%;

      print if($submit);
     }
   elsif (m%<!-- result-start -->%)
     {
      $results_section=1;
     }
   elsif (m%<!-- result-finish -->%)
     {
      $results_section=0;
     }
   elsif ($results_section)
     {
      s%<!-- result-Type -->%$router_Type%;
      s%<!-- result-type -->%$router_type%;
      s%<!-- result-uuid -->%$router_uuid%;
      s%<!-- result-time -->%$router_time%;
      s%<!-- result-result -->%$router_result%;
      s%<!-- result-message -->%$router_message%;

      print if($router_uuid);
     }
   else
     {
      print;
     }
  }

close(TEMPLATE);
