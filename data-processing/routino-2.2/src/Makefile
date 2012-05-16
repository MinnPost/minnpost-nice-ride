# Source code Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2012 Andrew M. Bishop
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

# Web file paths

WEBDIR=../web/bin

# Compilation programs

CC=gcc
LD=gcc

LEX=flex

# Compilation program options

CFLAGS=-Wall -Wmissing-prototypes -std=c99
#CFLAGS+=-Wextra -pedantic
LDFLAGS=-lm -lc

CFLAGS+=-O3
#CFLAGS+=-O0 -g
#CFLAGS+=-pg
#CFLAGS+=-fprofile-arcs -ftest-coverage

LDFLAGS+=
#LDFLAGS+=-pg -static
#LDFLAGS+=-fprofile-arcs -ftest-coverage

LEXFLAGS=

# Required to use stdio with files > 2GiB on 32-bit system.
CFLAGS+=-D_FILE_OFFSET_BITS=64

# Required to compile on Linux without a warning about pread() and pwrite() functions.
CFLAGS+=-D_POSIX_C_SOURCE=200809L

# Compilation targets

C=$(wildcard *.c)
D=$(wildcard .deps/*.d)

EXE=planetsplitter planetsplitter-slim router router-slim filedumper filedumper-slim tagmodifier

########

all: $(EXE)
	-@[ -d $(WEBDIR) ] && \
	  for file in $(EXE); do \
	     if [ ! -f $(WEBDIR)/$$file ] || [ $$file -nt $(WEBDIR)/$$file ]; then \
	        echo cp $$file $(WEBDIR) ;\
	        cp -f $$file $(WEBDIR) ;\
	     fi ;\
	  done
	@cd xml  && $(MAKE) CC="$(CC)" LD="$(LD)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"
	@cd test && $(MAKE) CC="$(CC)" LD="$(LD)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

########

PLANETSPLITTER_OBJ=planetsplitter.o \
	           nodesx.o segmentsx.o waysx.o relationsx.o superx.o prunex.o \
	           ways.o types.o \
	           files.o logging.o \
	           results.o queue.o sorting.o \
	           xmlparse.o tagging.o osmparser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

PLANETSPLITTER_SLIM_OBJ=planetsplitter-slim.o \
	                nodesx-slim.o segmentsx-slim.o waysx-slim.o relationsx-slim.o superx-slim.o prunex-slim.o \
	                ways.o types.o \
	                files.o logging.o \
	                results.o queue.o sorting.o \
	                xmlparse.o tagging.o osmparser.o

planetsplitter-slim : $(PLANETSPLITTER_SLIM_OBJ)
	$(LD) $(PLANETSPLITTER_SLIM_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o \
	   nodes.o segments.o ways.o relations.o types.o fakes.o \
	   optimiser.o output.o \
	   files.o logging.o profiles.o xmlparse.o \
	   results.o queue.o translations.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_SLIM_OBJ=router-slim.o \
	        nodes-slim.o segments-slim.o ways-slim.o relations-slim.o types.o fakes-slim.o \
	        optimiser-slim.o output-slim.o \
	        files.o logging.o profiles.o xmlparse.o \
	        results.o queue.o translations.o

router-slim : $(ROUTER_SLIM_OBJ)
	$(LD) $(ROUTER_SLIM_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o \
	       nodes.o segments.o ways.o relations.o types.o fakes.o \
               visualiser.o \
	       files.o logging.o xmlparse.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_SLIM_OBJ=filedumper-slim.o \
	       nodes-slim.o segments-slim.o ways-slim.o relations-slim.o types.o fakes-slim.o \
               visualiser-slim.o \
	       files.o logging.o xmlparse.o

filedumper-slim : $(FILEDUMPER_SLIM_OBJ)
	$(LD) $(FILEDUMPER_SLIM_OBJ) -o $@ $(LDFLAGS)

########

TAGMODIFIER_OBJ=tagmodifier.o \
	        files.o logging.o \
                xmlparse.o tagging.o

tagmodifier : $(TAGMODIFIER_OBJ)
	$(LD) $(TAGMODIFIER_OBJ) -o $@ $(LDFLAGS)

########

xmlparse.c : xmlparse.l
	$(LEX) $(LEXFLAGS)  $<
	-@mv lex.yy.c xmlparse.c
	@echo Created xmlparse.c

########

%.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=0 -DDATADIR=\"$(datadir)\" $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

%-slim.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=1 -DDATADIR=\"$(datadir)\" $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

########

test:
	cd xml  && $(MAKE) test
	cd test && $(MAKE) test

########

install: all
	-[ -d $(DESTDIR)$(bindir) ] || mkdir -p $(DESTDIR)$(bindir)
	@[ -d $(DESTDIR)$(bindir) ] && \
	  for file in $(EXE); do \
	     echo cp $$file $(DESTDIR)$(bindir) ;\
	     cp -f $$file $(DESTDIR)$(bindir) ;\
	  done

########

clean:
	rm -f *.o
	rm -f *~
	rm -f *.gcda *.gcno *.gcov gmon.out
	rm -f xmlparse.c
	cd xml  && $(MAKE) clean
	cd test && $(MAKE) clean

########

distclean: clean
	-[ -d ../web/bin ] && cd ../web/bin/ && rm -f $(EXE)
	-rm -f $(EXE)
	-rm -f $(D)
	-rm -fr .deps
	cd xml  && $(MAKE) distclean
	cd test && $(MAKE) distclean

########

include $(D)

########

top=-top
include ../Makefile
