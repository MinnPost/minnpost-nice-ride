#!/usr/bin/perl
#
# Routino data statistics
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2010 Andrew M. Bishop
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

# Use the perl CGI module
use CGI ':cgi';

# Print the output

print header('text/plain');

# Run the filedumper

$params.=" --dir=$data_dir" if($data_dir);
$params.=" --prefix=$data_prefix" if($data_prefix);
$params.=" --statistics";

system "$bin_dir/$filedumper_exe $params 2>&1";
