# XML test programs Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010-2011 Andrew M. Bishop
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

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-Wall -Wmissing-prototypes -std=c99
#CFLAGS+=-Wextra -pedantic
LDFLAGS=-lm -lc

CFLAGS+=-O3

# Required to use stdio with files > 2GiB on 32-bit system.
CFLAGS+=-D_FILE_OFFSET_BITS=64

# Compilation targets

XMLDIR=../../xml

X=$(notdir $(wildcard $(XMLDIR)/*.xsd))
C=$(foreach f,$(X),$(addsuffix -skeleton.c,$(basename $f)))
D=$(wildcard .deps/*.d)
O=$(foreach f,$(C),$(addsuffix .o,$(basename $f)))
E=$(foreach f,$(C),$(basename $f))

EXE=xsd-to-xmlparser

########

all: $(EXE) $(C) $(E)
	@true

########

xsd-to-xmlparser : xsd-to-xmlparser.o ../xmlparse.o
	$(LD) xsd-to-xmlparser.o ../xmlparse.o -o $@ $(LDFLAGS)

########

%-skeleton.c : $(XMLDIR)/%.xsd xsd-to-xmlparser
	-./xsd-to-xmlparser < $< > $@
	@test -s $@ || rm $@

%-skeleton : %-skeleton.o ../xmlparse.o
	$(LD) $< ../xmlparse.o -o $@ $(LDFLAGS)

.SECONDARY : $(O)

########

../xmlparse.o : ../xmlparse.c ../xmlparse.h
	cd .. && $(MAKE) xmlparse.o

../xmlparse.c : ../xmlparse.l
	cd .. && $(MAKE) xmlparse.c

########

%.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -I.. $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

########

test : all test-skeleton
	@status=true ;\
	echo "" ;\
	for good in test/good*.xml; do \
	   echo "Testing: $$good ... " ;\
	   if ./test-skeleton < $$good > /dev/null; then echo "... passed"; else echo "... FAILED"; status=false; fi ;\
	   echo "" ;\
	done ;\
	for bad in test/bad*.xml; do \
	   echo "Testing: $$bad ... " ;\
	   if ./test-skeleton < $$bad > /dev/null; then echo "... FAILED"; status=false; else echo "... passed"; fi ;\
	   echo "" ;\
	done ;\
	if $$status; then echo "Success: all tests passed"; else echo "Warning: Some tests FAILED"; fi ;\
	$$status

test-skeleton : test-skeleton.o
	$(LD) $< ../xmlparse.o -o $@ $(LDFLAGS)

test-skeleton.c : test/test.xsd xsd-to-xmlparser
	./xsd-to-xmlparser < $< | sed -e 's/XMLPARSE_UNKNOWN_ATTR_WARN/XMLPARSE_UNKNOWN_ATTR_ERROR/' > $@

########

install:

########

clean:
	rm -f *.o
	rm -f *~
	rm -f *.gcda *.gcno *.gcov gmon.out

########

distclean: clean
	-rm -f $(EXE)
	-rm -f $(E) test-skeleton
	-rm -f $(D) .deps/test-skeleton.d
	-rm -f $(C) test-skeleton.c
	-rm -fr .deps

########

include $(D)

########

.PHONY:: all test install clean distclean
