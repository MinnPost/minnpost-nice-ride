/***************************************
 Routing output generator.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2012 Andrew M. Bishop

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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "functions.h"
#include "fakes.h"
#include "translations.h"
#include "results.h"
#include "xmlparse.h"


/* Constants */

#define IMP_IGNORE      -1      /*+ Ignore this point. +*/
#define IMP_UNIMPORTANT  0      /*+ An unimportant, intermediate, node. +*/
#define IMP_RB_NOT_EXIT  1      /*+ A roundabout exit that is not taken. +*/
#define IMP_JUNCT_CONT   2      /*+ An un-interesting junction where the route continues without comment. +*/
#define IMP_CHANGE       3      /*+ The highway changes type but nothing else happens. +*/
#define IMP_JUNCT_IMPORT 4      /*+ An interesting junction to be described. +*/
#define IMP_RB_ENTRY     5      /*+ The entrance to a roundabout. +*/
#define IMP_RB_EXIT      6      /*+ The exit from a roundabout. +*/
#define IMP_MINI_RB      7      /*+ The location of a mini-roundabout. +*/
#define IMP_UTURN        8      /*+ The location of a U-turn. +*/
#define IMP_WAYPOINT     9      /*+ A waypoint. +*/


/* Global variables */

/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;

/*+ The options to select the format of the output. +*/
extern int option_html,option_gpx_track,option_gpx_route,option_text,option_text_all;


/* Local variables */

/*+ Heuristics for determining if a junction is important. +*/
static char junction_other_way[Way_Count][Way_Count]=
 { /* M, T, P, S, T, U, R, S, T, C, P, S, F = Way type of route not taken */
  {   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Motorway     */
  {   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Trunk        */
  {   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Primary      */
  {   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Secondary    */
  {   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Tertiary     */
  {   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 }, /* Unclassified */
  {   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 }, /* Residential  */
  {   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1 }, /* Service      */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1 }, /* Track        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 }, /* Cycleway     */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Path         */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Steps        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Ferry        */
 };


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results **results The set of results to print (some may be NULL - ignore them).

  int nresults The number of results in the list.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile)
{
 FILE *htmlfile=NULL,*gpxtrackfile=NULL,*gpxroutefile=NULL,*textfile=NULL,*textallfile=NULL;

 char *prev_bearing=NULL,*prev_wayname=NULL;
 distance_t cum_distance=0;
 duration_t cum_duration=0;

 int point=1;
 int segment_count=0,route_count=0;
 int point_count=0;
 int roundabout=0;

 /* Open the files */

 if(option_quickest==0)
   {
    /* Print the result for the shortest route */

    if(option_html)
       htmlfile    =fopen("shortest.html","w");
    if(option_gpx_track)
       gpxtrackfile=fopen("shortest-track.gpx","w");
    if(option_gpx_route)
       gpxroutefile=fopen("shortest-route.gpx","w");
    if(option_text)
       textfile    =fopen("shortest.txt","w");
    if(option_text_all)
       textallfile =fopen("shortest-all.txt","w");

    if(option_html && !htmlfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest.html' for writing [%s].\n",strerror(errno));
    if(option_gpx_track && !gpxtrackfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-track.gpx' for writing [%s].\n",strerror(errno));
    if(option_gpx_route && !gpxroutefile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-route.gpx' for writing [%s].\n",strerror(errno));
    if(option_text && !textfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest.txt' for writing [%s].\n",strerror(errno));
    if(option_text_all && !textallfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-all.txt' for writing [%s].\n",strerror(errno));
   }
 else
   {
    /* Print the result for the quickest route */

    if(option_html)
       htmlfile    =fopen("quickest.html","w");
    if(option_gpx_track)
       gpxtrackfile=fopen("quickest-track.gpx","w");
    if(option_gpx_route)
       gpxroutefile=fopen("quickest-route.gpx","w");
    if(option_text)
       textfile    =fopen("quickest.txt","w");
    if(option_text_all)
       textallfile =fopen("quickest-all.txt","w");

    if(option_html && !htmlfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest.html' for writing [%s].\n",strerror(errno));
    if(option_gpx_track && !gpxtrackfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-track.gpx' for writing [%s].\n",strerror(errno));
    if(option_gpx_route && !gpxroutefile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-route.gpx' for writing [%s].\n",strerror(errno));
    if(option_text && !textfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest.txt' for writing [%s].\n",strerror(errno));
    if(option_text_all && !textallfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-all.txt' for writing [%s].\n",strerror(errno));
   }

 /* Print the head of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
    fprintf(htmlfile,"<HTML>\n");
    if(translate_xml_copyright_creator[0] && translate_xml_copyright_creator[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_xml_copyright_creator[0],translate_xml_copyright_creator[1]);
    if(translate_xml_copyright_source[0] && translate_xml_copyright_source[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_xml_copyright_source[0],translate_xml_copyright_source[1]);
    if(translate_xml_copyright_license[0] && translate_xml_copyright_license[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_xml_copyright_license[0],translate_xml_copyright_license[1]);
    fprintf(htmlfile,"<HEAD>\n");
    fprintf(htmlfile,"<TITLE>");
    fprintf(htmlfile,translate_html_title,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(htmlfile,"</TITLE>\n");
    fprintf(htmlfile,"<META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
    fprintf(htmlfile,"<STYLE type=\"text/css\">\n");
    fprintf(htmlfile,"<!--\n");
    fprintf(htmlfile,"   table   {table-layout: fixed; border: none; border-collapse: collapse;}\n");
    fprintf(htmlfile,"   table.c {color: grey; font-size: x-small;} /* copyright */\n");
    fprintf(htmlfile,"   tr      {border: 0px;}\n");
    fprintf(htmlfile,"   tr.c    {display: none;} /* coords */\n");
    fprintf(htmlfile,"   tr.n    {} /* node */\n");
    fprintf(htmlfile,"   tr.s    {} /* segment */\n");
    fprintf(htmlfile,"   tr.t    {font-weight: bold;} /* total */\n");
    fprintf(htmlfile,"   td.l    {font-weight: bold;}\n");
    fprintf(htmlfile,"   td.r    {}\n");
    fprintf(htmlfile,"   span.w  {font-weight: bold;} /* waypoint */\n");
    fprintf(htmlfile,"   span.h  {text-decoration: underline;} /* highway */\n");
    fprintf(htmlfile,"   span.d  {} /* segment distance */\n");
    fprintf(htmlfile,"   span.j  {font-style: italic;} /* total journey distance */\n");
    fprintf(htmlfile,"   span.t  {font-variant: small-caps;} /* turn */\n");
    fprintf(htmlfile,"   span.b  {font-variant: small-caps;} /* bearing */\n");
    fprintf(htmlfile,"-->\n");
    fprintf(htmlfile,"</STYLE>\n");
    fprintf(htmlfile,"</HEAD>\n");
    fprintf(htmlfile,"<BODY>\n");
    fprintf(htmlfile,"<H1>");
    fprintf(htmlfile,translate_html_title,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(htmlfile,"</H1>\n");
    fprintf(htmlfile,"<table>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxtrackfile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxtrackfile,"<metadata>\n");
    fprintf(gpxtrackfile,"<desc>%s : %s</desc>\n",translate_xml_copyright_creator[0],translate_xml_copyright_creator[1]);
    if(translate_xml_copyright_source[1])
      {
       fprintf(gpxtrackfile,"<copyright author=\"%s\">\n",translate_xml_copyright_source[1]);

       if(translate_xml_copyright_license[1])
          fprintf(gpxtrackfile,"<license>%s</license>\n",translate_xml_copyright_license[1]);

       fprintf(gpxtrackfile,"</copyright>\n");
      }
    fprintf(gpxtrackfile,"</metadata>\n");

    fprintf(gpxtrackfile,"<trk>\n");
    fprintf(gpxtrackfile,"<name>");
    fprintf(gpxtrackfile,translate_gpx_name,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(gpxtrackfile,"</name>\n");
    fprintf(gpxtrackfile,"<desc>");
    fprintf(gpxtrackfile,translate_gpx_desc,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(gpxtrackfile,"</desc>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxroutefile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxroutefile,"<metadata>\n");
    fprintf(gpxroutefile,"<desc>%s : %s</desc>\n",translate_xml_copyright_creator[0],translate_xml_copyright_creator[1]);
    if(translate_xml_copyright_source[1])
      {
       fprintf(gpxroutefile,"<copyright author=\"%s\">\n",translate_xml_copyright_source[1]);

       if(translate_xml_copyright_license[1])
          fprintf(gpxroutefile,"<license>%s</license>\n",translate_xml_copyright_license[1]);

       fprintf(gpxroutefile,"</copyright>\n");
      }
    fprintf(gpxroutefile,"</metadata>\n");

    fprintf(gpxroutefile,"<rte>\n");
    fprintf(gpxroutefile,"<name>");
    fprintf(gpxroutefile,translate_gpx_name,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(gpxroutefile,"</name>\n");
    fprintf(gpxroutefile,"<desc>");
    fprintf(gpxroutefile,translate_gpx_desc,option_quickest?translate_xml_route_quickest:translate_xml_route_shortest);
    fprintf(gpxroutefile,"</desc>\n");
   }

 if(textfile)
   {
    if(translate_raw_copyright_creator[0] && translate_raw_copyright_creator[1])
       fprintf(textfile,"# %s : %s\n",translate_raw_copyright_creator[0],translate_raw_copyright_creator[1]);
    if(translate_raw_copyright_source[0] && translate_raw_copyright_source[1])
       fprintf(textfile,"# %s : %s\n",translate_raw_copyright_source[0],translate_raw_copyright_source[1]);
    if(translate_raw_copyright_license[0] && translate_raw_copyright_license[1])
       fprintf(textfile,"# %s : %s\n",translate_raw_copyright_license[0],translate_raw_copyright_license[1]);
    if((translate_raw_copyright_creator[0] && translate_raw_copyright_creator[1]) ||
       (translate_raw_copyright_source[0]  && translate_raw_copyright_source[1]) ||
       (translate_raw_copyright_license[0] && translate_raw_copyright_license[1]))
       fprintf(textfile,"#\n");

    fprintf(textfile,"#Latitude\tLongitude\tSection \tSection \tTotal   \tTotal   \tPoint\tTurn\tBearing\tHighway\n");
    fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDuration\tType \t    \t       \t       \n");
                     /* "%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n" */
   }

 if(textallfile)
   {
    if(translate_raw_copyright_creator[0] && translate_raw_copyright_creator[1])
       fprintf(textallfile,"# %s : %s\n",translate_raw_copyright_creator[0],translate_raw_copyright_creator[1]);
    if(translate_raw_copyright_source[0] && translate_raw_copyright_source[1])
       fprintf(textallfile,"# %s : %s\n",translate_raw_copyright_source[0],translate_raw_copyright_source[1]);
    if(translate_raw_copyright_license[0] && translate_raw_copyright_license[1])
       fprintf(textallfile,"# %s : %s\n",translate_raw_copyright_license[0],translate_raw_copyright_license[1]);
    if((translate_raw_copyright_creator[0] && translate_raw_copyright_creator[1]) ||
       (translate_raw_copyright_source[0]  && translate_raw_copyright_source[1]) ||
       (translate_raw_copyright_license[0] && translate_raw_copyright_license[1]))
       fprintf(textallfile,"#\n");

    fprintf(textallfile,"#Latitude\tLongitude\t    Node\tType\tSegment\tSegment\tTotal\tTotal  \tSpeed\tBearing\tHighway\n");
    fprintf(textallfile,"#        \t         \t        \t    \tDist   \tDurat'n\tDist \tDurat'n\t     \t       \t       \n");
                        /* "%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n" */
   }

 /* Loop through all the sections of the route and print them */

 while(!results[point])
    point++;

 while(point<=nresults)
   {
    int next_point=point;
    distance_t junc_distance=0;
    duration_t junc_duration=0;
    Result *result;

    result=FindResult(results[point],results[point]->start_node,results[point]->prev_segment);

    /* Print the start of the segment */

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"<trkseg>\n");

    /* Loop through all the points within a section of the route and print them */

    do
      {
       double latitude,longitude;
       Node *resultnode=NULL;
       index_t realsegment=NO_SEGMENT,next_realsegment=NO_SEGMENT;
       Segment *resultsegment=NULL,*next_resultsegment=NULL;
       Way *resultway=NULL,*next_resultway=NULL;
       Result *next_result;
       int important=IMP_UNIMPORTANT;

       distance_t seg_distance=0;
       duration_t seg_duration=0;
       speed_t seg_speed=0;
       char *waynameraw=NULL,*wayname=NULL,*next_waynameraw=NULL,*next_wayname=NULL;
       int bearing_int=0,turn_int=0,next_bearing_int=0;
       char *turn=NULL,*next_bearing=NULL;

       /* Calculate the information about this point */

       if(IsFakeNode(result->node))
          GetFakeLatLong(result->node,&latitude,&longitude);
       else
          GetLatLong(nodes,result->node,&latitude,&longitude);

       if(!IsFakeNode(result->node))
          resultnode=LookupNode(nodes,result->node,6);

       /* Calculate the next result */

       next_result=result->next;

       if(!next_result)
          for(next_point=point+1;next_point<=nresults;next_point++)
             if(results[next_point])
               {
                next_result=FindResult(results[next_point],results[next_point]->start_node,results[next_point]->prev_segment);
                next_result=next_result->next;
                break;
               }

       /* Calculate the information about this segment */

       if(result->node!=results[point]->start_node) /* not first point of a section of the route */
         {
          if(IsFakeSegment(result->segment))
            {
             resultsegment=LookupFakeSegment(result->segment);
             realsegment=IndexRealSegment(result->segment);
            }
          else
            {
             resultsegment=LookupSegment(segments,result->segment,2);
             realsegment=result->segment;
            }

          resultway=LookupWay(ways,resultsegment->way,1);

          seg_distance+=DISTANCE(resultsegment->distance);
          seg_duration+=Duration(resultsegment,resultway,profile);

          /* Calculate the cumulative distance/duration */

          junc_distance+=seg_distance;
          junc_duration+=seg_duration;
          cum_distance+=seg_distance;
          cum_duration+=seg_duration;
         }

       /* Calculate the information about the next segment */

       if(next_result)
         {
          if(IsFakeSegment(next_result->segment))
            {
             next_resultsegment=LookupFakeSegment(next_result->segment);
             next_realsegment=IndexRealSegment(next_result->segment);
            }
          else
            {
             next_resultsegment=LookupSegment(segments,next_result->segment,1);
             next_realsegment=next_result->segment;
            }
         }

       /* Decide if this is a roundabout */

       if(next_result)
         {
          next_resultway=LookupWay(ways,next_resultsegment->way,2);

          if(next_resultway->type&Way_Roundabout)
            {
             if(roundabout==0)
               {
                roundabout++;
                important=IMP_RB_ENTRY;
               }
             else
               {
                Segment *segment=FirstSegment(segments,resultnode,3);

                do
                  {
                   index_t othernode=OtherNode(segment,result->node);

                   if(othernode!=result->prev->node && IndexSegment(segments,segment)!=realsegment)
                      if(IsNormalSegment(segment) && (!profile->oneway || !IsOnewayTo(segment,result->node)))
                        {
                         Way *way=LookupWay(ways,segment->way,3);

                         if(!(way->type&Way_Roundabout))
                            if(othernode!=next_result->node)
                              {
                               roundabout++;
                               important=IMP_RB_NOT_EXIT;
                              }
                        }

                   segment=NextSegment(segments,segment,result->node);
                  }
                while(segment);
               }
            }
          else
             if(roundabout)
               {
                roundabout++;
                important=IMP_RB_EXIT;
               }
         }

       /* Decide if this is an important junction */

       if(roundabout)           /* roundabout */
          ;
       else if(point_count==0)  /* first point overall = Waypoint */
          important=IMP_WAYPOINT;
       else if(result->node==results[point]->finish_node) /* Waypoint */
          important=IMP_WAYPOINT;
       else if(result->node==results[point]->start_node) /* first point of a section of the route */
          important=IMP_IGNORE;
       else if(realsegment==next_realsegment) /* U-turn */
          important=IMP_UTURN;
       else if(resultnode && (resultnode->flags&NODE_MINIRNDBT))
          important=IMP_MINI_RB; /* mini-roundabout */
       else
         {
          Segment *segment=FirstSegment(segments,resultnode,3);

          do
            {
             index_t othernode=OtherNode(segment,result->node);

             if(othernode!=result->prev->node && IndexSegment(segments,segment)!=realsegment)
                if(IsNormalSegment(segment) && (!profile->oneway || !IsOnewayTo(segment,result->node)))
                  {
                   Way *way=LookupWay(ways,segment->way,3);

                   if(othernode==next_result->node) /* the next segment that we follow */
                     {
                      if(HIGHWAY(way->type)!=HIGHWAY(resultway->type))
                         if(important<IMP_CHANGE)
                            important=IMP_CHANGE;
                     }
                   else if(IsFakeNode(next_result->node))
                      ;
                   else /* a segment that we don't follow */
                     {
                      if(junction_other_way[HIGHWAY(resultway->type)-1][HIGHWAY(way->type)-1])
                         if(important<IMP_JUNCT_IMPORT)
                            important=IMP_JUNCT_IMPORT;

                      if(important<IMP_JUNCT_CONT)
                         important=IMP_JUNCT_CONT;
                     }
                  }

             segment=NextSegment(segments,segment,result->node);
            }
          while(segment);
         }

       /* Calculate the strings to be used */

       if(resultway && textallfile)
         {
          waynameraw=WayName(ways,resultway);
          if(!*waynameraw)
             waynameraw=translate_raw_highway[HIGHWAY(resultway->type)];

          bearing_int=(int)BearingAngle(nodes,resultsegment,result->node);

          seg_speed=profile->speed[HIGHWAY(resultway->type)];
         }

       if(next_result && important>IMP_JUNCT_CONT)
         {
          if(resultsegment && (htmlfile || textfile))
            {
             turn_int=(int)TurnAngle(nodes,resultsegment,next_resultsegment,result->node);
             turn=translate_xml_turn[((202+turn_int)/45)%8];
            }

          if(gpxroutefile || htmlfile)
            {
             next_waynameraw=WayName(ways,next_resultway);
             if(!*next_waynameraw)
                next_waynameraw=translate_raw_highway[HIGHWAY(next_resultway->type)];

             next_wayname=ParseXML_Encode_Safe_XML(next_waynameraw);
            }

          if(htmlfile || gpxroutefile || textfile)
            {
             next_bearing_int=(int)BearingAngle(nodes,next_resultsegment,next_result->node);
             next_bearing=translate_xml_heading[(4+(22+next_bearing_int)/45)%8];
            }
         }

       /* Print out the important points (junctions / waypoints) */

       if(important>IMP_JUNCT_CONT)
         {
          if(htmlfile)
            {
             char *type;

             if(important==IMP_WAYPOINT)
                type=translate_html_waypoint;
             else if(important==IMP_MINI_RB)
                type=translate_html_roundabout;
             else
                type=translate_html_junction;

             if(point_count>0)  /* not the first point */
               {
                /* <tr class='s'><td class='l'>Follow:<td class='r'><span class='h'>*highway name*</span> for <span class='d'>*distance* km, *time* min</span> [<span class='j'>*distance* km, *time* minutes</span>] */
                fprintf(htmlfile,"<tr class='s'><td class='l'>%s:<td class='r'>",translate_html_segment[0]);
                fprintf(htmlfile,translate_html_segment[1],
                                 (roundabout>1?translate_html_roundabout:prev_wayname),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(htmlfile," [<span class='j'>");
                fprintf(htmlfile,translate_html_total[1],
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                fprintf(htmlfile,"</span>]\n");
               }

             /* <tr class='c'><td class='l'>*N*:<td class='r'>*latitude* *longitude* */
             fprintf(htmlfile,"<tr class='c'><td class='l'>%d:<td class='r'>%.6f %.6f\n",
                              point_count+1,
                              radians_to_degrees(latitude),radians_to_degrees(longitude));

             if(point_count==0) /* first point */
               {
                /* <tr class='n'><td class='l'>Start:<td class='r'>At <span class='w'>Waypoint</span>, head <span class='b'>*heading*</span> */
                fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_start[0]);
                fprintf(htmlfile,translate_html_start[1],
                                 translate_html_waypoint,
                                 next_bearing);
                fprintf(htmlfile,"\n");
               }
             else if(next_result) /* middle point */
               {
                if(roundabout>1)
                  {
                   /* <tr class='n'><td class='l'>At:<td class='r'>Roundabout, take <span class='t'>the *Nth* exit</span> heading <span class='b'>*heading*</span> */
                   fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_rbnode[0]);
                   fprintf(htmlfile,translate_html_rbnode[1],
                                    translate_html_roundabout,
                                    translate_xml_ordinal[roundabout-2],
                                    next_bearing);
                   fprintf(htmlfile,"\n");
                  }
                else
                  {
                   /* <tr class='n'><td class='l'>At:<td class='r'>Junction, go <span class='t'>*direction*</span> heading <span class='b'>*heading*</span> */
                   fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_node[0]);
                   fprintf(htmlfile,translate_html_node[1],
                                    type,
                                    turn,
                                    next_bearing);
                   fprintf(htmlfile,"\n");
                  }
               }
             else            /* end point */
               {
                /* <tr class='n'><td class='l'>Stop:<td class='r'>At <span class='w'>Waypoint</span> */
                fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_stop[0]);
                fprintf(htmlfile,translate_html_stop[1],
                                 translate_html_waypoint);
                fprintf(htmlfile,"\n");

                /* <tr class='t'><td class='l'>Total:<td class='r'><span class='j'>*distance* km, *time* minutes</span> */
                fprintf(htmlfile,"<tr class='t'><td class='l'>%s:<td class='r'><span class='j'>",translate_html_total[0]);
                fprintf(htmlfile,translate_html_total[1],
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                fprintf(htmlfile,"</span>\n");
               }
            }

          if(gpxroutefile)
            {
             if(point_count>0) /* not first point */
               {
                fprintf(gpxroutefile,"<desc>");
                fprintf(gpxroutefile,translate_gpx_step,
                                     prev_bearing,
                                     prev_wayname,
                                     distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(gpxroutefile,"</desc></rtept>\n");
               }

             if(point_count==0) /* first point */
               {
                fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                     radians_to_degrees(latitude),radians_to_degrees(longitude),
                                     translate_gpx_start);
               }
             else if(!next_result) /* end point */
               {
                fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                     radians_to_degrees(latitude),radians_to_degrees(longitude),
                                     translate_gpx_finish);
                fprintf(gpxroutefile,"<desc>");
                fprintf(gpxroutefile,translate_gpx_final,
                                     distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                fprintf(gpxroutefile,"</desc></rtept>\n");
               }
             else            /* middle point */
               {
                if(important==IMP_WAYPOINT)
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translate_gpx_inter,++segment_count);
                else
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%03d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translate_gpx_trip,++route_count);
               }
            }

          if(textfile)
            {
             char *type;

             if(important==IMP_WAYPOINT)
                type="Waypt";
             else
                type="Junct";

             if(point_count==0) /* first point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t %+d\t%s\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 0.0,0.0,0.0,0.0,
                                 type,
                                 ((22+next_bearing_int)/45+4)%8-4,
                                 next_waynameraw);
               }
             else if(!next_result) /* end point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t\t\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                 type);
               }
             else               /* middle point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                 type,
                                 (22+turn_int)/45,
                                 ((22+next_bearing_int)/45+4)%8-4,
                                 next_waynameraw);
               }
            }

          junc_distance=0;
          junc_duration=0;

          if(htmlfile || gpxroutefile)
            {
             if(prev_wayname)
                free(prev_wayname);

             if(next_wayname)
                prev_wayname=strcpy((char*)malloc(strlen(next_wayname)+1),next_wayname);
             else
                prev_wayname=NULL;

             if(next_wayname && next_wayname!=next_waynameraw)
                free(next_wayname);
            }

          if(gpxroutefile)
             prev_bearing=next_bearing;

          if(roundabout>1)
             roundabout=0;
         }

       /* Print out all of the results */

       if(gpxtrackfile)
          fprintf(gpxtrackfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"/>\n",
                               radians_to_degrees(latitude),radians_to_degrees(longitude));

       if(important>IMP_IGNORE)
         {
          if(textallfile)
            {
             char *type;

             if(important==IMP_WAYPOINT)
                type="Waypt";
             else if(important==IMP_UTURN)
                type="U-turn";
             else if(important==IMP_MINI_RB)
                type="Mini-RB";
             else if(important==IMP_CHANGE)
                type="Change";
             else if(important==IMP_UNIMPORTANT)
                type="Inter";
             else
                type="Junct";

             if(point_count==0) /* first point */
               {
                fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t\t\t\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    IsFakeNode(result->node)?(NODE_FAKE-result->node):result->node,
                                    (resultnode && IsSuperNode(resultnode))?'*':' ',type,
                                    0.0,0.0,0.0,0.0);
               }
             else               /* not the first point */
               {
                fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    IsFakeNode(result->node)?(NODE_FAKE-result->node):result->node,
                                    (resultnode && IsSuperNode(resultnode))?'*':' ',type,
                                    distance_to_km(seg_distance),duration_to_minutes(seg_duration),
                                    distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                    speed_to_kph(seg_speed),
                                    bearing_int,
                                    waynameraw);
               }
            }
         }

       if(wayname && wayname!=waynameraw)
          free(wayname);

       result=next_result;

       if(important>IMP_JUNCT_CONT)
          point_count++;
      }
    while(point==next_point);

    /* Print the end of the segment */

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"</trkseg>\n");

    point=next_point;
   }

 /* Print the tail of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"</table>\n");

    if((translate_xml_copyright_creator[0] && translate_xml_copyright_creator[1]) ||
       (translate_xml_copyright_source[0]  && translate_xml_copyright_source[1]) ||
       (translate_xml_copyright_license[0] && translate_xml_copyright_license[1]))
      {
       fprintf(htmlfile,"<p>\n");
       fprintf(htmlfile,"<table class='c'>\n");
       if(translate_xml_copyright_creator[0] && translate_xml_copyright_creator[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_xml_copyright_creator[0],translate_xml_copyright_creator[1]);
       if(translate_xml_copyright_source[0] && translate_xml_copyright_source[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_xml_copyright_source[0],translate_xml_copyright_source[1]);
       if(translate_xml_copyright_license[0] && translate_xml_copyright_license[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_xml_copyright_license[0],translate_xml_copyright_license[1]);
       fprintf(htmlfile,"</table>\n");
      }

    fprintf(htmlfile,"</BODY>\n");
    fprintf(htmlfile,"</HTML>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"</trk>\n");
    fprintf(gpxtrackfile,"</gpx>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"</rte>\n");
    fprintf(gpxroutefile,"</gpx>\n");
   }

 /* Close the files */

 if(htmlfile)
    fclose(htmlfile);
 if(gpxtrackfile)
    fclose(gpxtrackfile);
 if(gpxroutefile)
    fclose(gpxroutefile);
 if(textfile)
    fclose(textfile);
 if(textallfile)
    fclose(textallfile);
}
