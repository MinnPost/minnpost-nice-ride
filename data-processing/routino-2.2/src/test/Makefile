# Test cases Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2011-2012 Andrew M. Bishop
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

# Routino executables

EXE=../planetsplitter ../planetsplitter-slim \
    ../router ../router-slim \
    ../filedumper ../filedumper-slim

# Compilation targets

O=$(notdir $(wildcard *.osm))
S=$(foreach f,$(O),$(addsuffix .sh,$(basename $f)))

########

all :
	@true

########

exe :
	cd .. && $(MAKE)

########

test : exe
	@status=true ;\
	for script in $(S); do \
	   echo "" ;\
	   echo "Testing: $$script (non-slim, no pruning) ... " ;\
	   if ./$$script fat; then echo "... passed"; else echo "... FAILED"; status=false; fi ;\
	done ;\
	for script in $(S); do \
	   echo "" ;\
	   echo "Testing: $$script (slimm, no pruning) ... " ;\
	   if ./$$script slim; then echo "... passed"; else echo "... FAILED"; status=false; fi ;\
	done ;\
	echo "" ;\
	if $$status; then echo "Success: all tests passed"; else echo "Warning: Some tests FAILED"; fi ;\
	$$status || exit 1 ;\
	echo "" ;\
	echo "Comparing: slim and non-slim results ... " ;\
	if diff -q -r slim fat; then echo "... matched"; else echo "... match FAILED"; status=false; fi ;\
	echo "" ;\
	if $$status; then echo "Success: slim and non-slim results match"; else echo "Warning: slim and non-slim results are different - FAILED"; fi ;\
	for script in $(S); do \
	   echo "" ;\
	   echo "Testing: $$script (non-slim, pruning) ... " ;\
	   if ./$$script fat prune; then echo "... passed"; else echo "... FAILED"; status=false; fi ;\
	done ;\
	for script in $(S); do \
	   echo "" ;\
	   echo "Testing: $$script (slim, pruning) ... " ;\
	   if ./$$script slim prune; then echo "... passed"; else echo "... FAILED"; status=false; fi ;\
	done ;\
	echo "" ;\
	if $$status; then echo "Success: all tests passed"; else echo "Warning: Some tests FAILED"; fi ;\
	$$status || exit 1 ;\
	echo "" ;\
	echo "Comparing: slim and non-slim results ... " ;\
	if diff -q -r slim-pruned fat-pruned; then echo "... matched"; else echo "... match FAILED"; status=false; fi ;\
	echo "" ;\
	if $$status; then echo "Success: slim and non-slim results match"; else echo "Warning: slim and non-slim results are different - FAILED"; fi ;\
	$$status

########

clean:
	rm -rf fat
	rm -rf slim
	rm -rf fat-pruned
	rm -rf slim-pruned
	rm -f *.log
	rm -f core
	rm -f *~

########

distclean: clean
	@true

########

.PHONY:: all test install clean distclean
