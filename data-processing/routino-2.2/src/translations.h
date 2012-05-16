/***************************************
 Load the translations from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2011 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Global variable declarations */

extern char *translate_raw_copyright_creator[2];
extern char *translate_raw_copyright_source[2];
extern char *translate_raw_copyright_license[2];

extern char *translate_xml_copyright_creator[2];
extern char *translate_xml_copyright_source[2];
extern char *translate_xml_copyright_license[2];

extern char *translate_xml_heading[9];
extern char *translate_xml_turn[9];
extern char *translate_xml_ordinal[10];

extern char *translate_raw_highway[Way_Count];

extern char *translate_xml_route_shortest;
extern char *translate_xml_route_quickest;

extern char *translate_html_waypoint;
extern char *translate_html_junction;
extern char *translate_html_roundabout;

extern char *translate_html_title;
extern char *translate_html_start[2];
extern char *translate_html_segment[2];
extern char *translate_html_node[2];
extern char *translate_html_rbnode[2];
extern char *translate_html_stop[2];
extern char *translate_html_total[2];

extern char *translate_gpx_desc;
extern char *translate_gpx_name;
extern char *translate_gpx_step;
extern char *translate_gpx_final;

extern char *translate_gpx_start;
extern char *translate_gpx_inter;
extern char *translate_gpx_trip;
extern char *translate_gpx_finish;


/* Functions in translations.c */

int ParseXMLTranslations(const char *filename,const char *language);


#endif /* TRANSLATIONS_H */
