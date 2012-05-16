# Documentation directory Makefile
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

# Web file paths

WEBDIR=../web/www/routino/documentation

# Files to install

HTML_FILES=html/*
TXT_FILES=*.txt
TOP_FILES=../agpl-3.0.txt

########

all:
	-@[ -d $(WEBDIR) ] && \
	  for file in $(HTML_FILES); do \
	     if [ ! -f $(WEBDIR)/`basename $$file` ] || [ $$file -nt $(WEBDIR)/`basename $$file` ]; then \
	        echo cp $$file $(WEBDIR) ;\
	        cp -f $$file $(WEBDIR) ;\
	     fi ;\
	  done

########

test:

########

install: all
	-[ -d $(DESTDIR)$(docdir) ] || mkdir -p $(DESTDIR)$(docdir)
	@[ -d $(DESTDIR)$(docdir) ] && \
	  for file in $(TOP_FILES); do \
	     echo cp $$file $(DESTDIR)$(docdir) ;\
	     cp -f $$file $(DESTDIR)$(docdir) ;\
	  done
	@[ -d $(DESTDIR)$(docdir) ] && \
	  for file in $(TXT_FILES); do \
	     echo cp $$file $(DESTDIR)$(docdir) ;\
	     cp -f $$file $(DESTDIR)$(docdir) ;\
	  done
	-[ -d $(DESTDIR)$(docdir)/html ] || mkdir -p $(DESTDIR)$(docdir)/html
	@[ -d $(DESTDIR)$(docdir)/html ] && \
	  for file in $(HTML_FILES); do \
	     echo cp $$file $(DESTDIR)$(docdir)/html ;\
	     cp -f $$file $(DESTDIR)$(docdir)/html ;\
	  done

########

clean:
	rm -f *~
	rm -f html/*~
	rm -f $(WEBDIR)/*~

########

distclean: clean
	rm -f $(WEBDIR)/*

########

top=-top
include ../Makefile
