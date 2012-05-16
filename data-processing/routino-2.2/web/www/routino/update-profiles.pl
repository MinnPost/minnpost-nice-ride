#!/usr/bin/perl
#
# Update the Routino profile files
#
# Part of the Routino routing software.
#
# This file Copyright 2011 Andrew M. Bishop
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


# The parameters for the execution

$params.=" --dir=$data_dir" if($data_dir);
$params.=" --prefix=$data_prefix" if($data_prefix);


# Generate the Perl profiles.

open(PROFILE,">profiles.pl") || die "Cannot open 'profiles.pl' to write.\n";

print PROFILE "################################################################################\n";
print PROFILE "########################### Routino default profile ############################\n";
print PROFILE "################################################################################\n";
print PROFILE "\n";

open(EXECUTE,"$bin_dir/$router_exe $params --help-profile-perl |") || die "Failed to execute router to generate profiles.\n";

while(<EXECUTE>)
  {
   print PROFILE;
  }

close(EXECUTE);

print PROFILE "\n";
print PROFILE "1;\n";

close(PROFILE);


# Generate the Javascript profiles.

open(PROFILE,">profiles.js") || die "Cannot open 'profiles.js' to write.\n";

print PROFILE "////////////////////////////////////////////////////////////////////////////////\n";
print PROFILE "/////////////////////////// Routino default profile ////////////////////////////\n";
print PROFILE "////////////////////////////////////////////////////////////////////////////////\n";
print PROFILE "\n";

open(EXECUTE,"$bin_dir/$router_exe $params --help-profile-json |") || die "Failed to execute router to generate profiles.\n";

while(<EXECUTE>)
  {
   print PROFILE;
  }

close(EXECUTE);

close(PROFILE);
