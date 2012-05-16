#
# Routino CGI paths Perl script
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

# Directory path parameters

# EDIT THIS to set the root directory for the non-web data files.
$root_dir="../..";

# EDIT THIS to change the location of the individual directories.
$bin_dir="$root_dir/bin";
$data_dir="$root_dir/data";
$results_dir="$root_dir/results";

# EDIT THIS to change the names of the executables (enables easy selection of slim mode).
$router_exe="router";
$filedumper_exe="filedumper";

1;
