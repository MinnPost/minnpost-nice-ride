# XML directory Makefile
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

WEBDATADIR=../web/data
WEBWWWDIR=../web/www/routino

# Files to install

STANDARD_FILES=profiles.xml \
               translations.xml \
               tagging.xml

SPECIAL_FILES=tagging-drive.xml \
	      tagging-ride.xml \
              tagging-walk.xml

PROFILE_FILES=$(WEBWWWDIR)/profiles.pl \
	      $(WEBWWWDIR)/profiles.js

########

all: $(SPECIAL_FILES)
	-@for file in $(STANDARD_FILES); do \
	     if [ ! -f $(WEBDATADIR)/$$file ] || [ routino-$$file -nt $(WEBDATADIR)/$$file ]; then \
	        echo cp routino-$$file $(WEBDATADIR)/$$file ;\
	        cp -f routino-$$file $(WEBDATADIR)/$$file ;\
	     fi ;\
	  done
	-@for file in $(SPECIAL_FILES); do \
	     if [ ! -f $(WEBDATADIR)/$$file ] || [ $$file -nt $(WEBDATADIR)/$$file ]; then \
	        echo cp $$file $(WEBDATADIR)/$$file ;\
	        cp -f $$file $(WEBDATADIR)/$$file ;\
	     fi ;\
	  done
	-@if [ ! -f $(WEBWWWDIR)/profiles.js ] || [ ! -f $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.js ]; then \
	     echo perl update-profiles.pl ;\
	     ( cd $(WEBWWWDIR) ; perl update-profiles.pl ) ;\
	  fi

####

tagging-drive.xml : routino-tagging.xml scripts/drive.pl
	perl scripts/drive.pl < routino-tagging.xml > tagging-drive.xml

tagging-ride.xml : routino-tagging.xml scripts/ride.pl
	perl scripts/ride.pl < routino-tagging.xml > tagging-ride.xml

tagging-walk.xml : routino-tagging.xml scripts/walk.pl
	perl scripts/walk.pl < routino-tagging.xml > tagging-walk.xml

########

test:

########

install: all
	-[ -d $(DESTDIR)$(datadir) ] || mkdir -p $(DESTDIR)$(datadir)
	-@for file in $(STANDARD_FILES) ; do \
	     echo cp routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	     cp -f routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	  done
	-@for file in $(SPECIAL_FILES); do \
	     echo cp $$file $(DESTDIR)$(datadir)/$$file ;\
	     cp -f $$file $(DESTDIR)$(datadir)/$$file ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
	rm -f $(WEBDATADIR)/*.xml
	rm -f $(SPECIAL_FILES)
	rm -f $(PROFILE_FILES)

########

top=-top
include ../Makefile
