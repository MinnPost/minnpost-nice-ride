# Top level Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2009-2011 Andrew M. Bishop
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

# Installation locations

prefix=/usr/local
bindir=$(prefix)/bin
docdir=$(prefix)/doc/routino
datadir=$(prefix)/share/routino

# Sub-directories and sub-makefiles

TOPFILES=$(wildcard */Makefile)
TOPDIRS=$(foreach f,$(TOPFILES),$(dir $f))

########

all$(top):
	for dir in $(TOPDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

########

test$(top):
	for dir in $(TOPDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

########

install$(top): all$(top)
	for dir in $(TOPDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done
	@echo "Note: web directory is not installed automatically"

########

clean$(top):
	for dir in $(TOPDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

########

distclean$(top): clean$(top)
	for dir in $(TOPDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

########

.PHONY:: all$(top) test$(top) install$(top) clean$(top) distclean$(top)

.PHONY:: all test install clean distclean
