/***************************************
 Extract data from Routino.

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "visualiser.h"


/*+ The maximum number of segments per node (used to size temporary storage). +*/
#define MAX_SEG_PER_NODE 32

/* Limit types */

#define SPEED_LIMIT  1
#define WEIGHT_LIMIT 2
#define HEIGHT_LIMIT 3
#define WIDTH_LIMIT  4
#define LENGTH_LIMIT 5

/* Local types */

typedef void (*callback_t)(index_t node,double latitude,double longitude);

/* Local variables */

static Nodes     *OSMNodes;
static Segments  *OSMSegments;
static Ways      *OSMWays;
static Relations *OSMRelations;

static double LatMin;
static double LatMax;
static double LonMin;
static double LonMax;

static int limit_type=0;

/* Local functions */

static void find_all_nodes(Nodes *nodes,callback_t callback);
static void output_junctions(index_t node,double latitude,double longitude);
static void output_super(index_t node,double latitude,double longitude);
static void output_oneway(index_t node,double latitude,double longitude);
static void output_turnrestriction(index_t node,double latitude,double longitude);
static void output_limits(index_t node,double latitude,double longitude);


/*++++++++++++++++++++++++++++++++++++++
  Output the data for junctions.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputJunctions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_junctions);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node as a junction (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_junctions(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segment;
 Way *firstway;
 int count=0,difference=0;

 segment=FirstSegment(OSMSegments,nodep,1);
 firstway=LookupWay(OSMWays,segment->way,1);

 do
   {
    Way *way=LookupWay(OSMWays,segment->way,2);

    if(IsNormalSegment(segment))
       count++;

    if(WaysCompare(firstway,way))
       difference=1;

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);

 if(count!=2 || difference)
    printf("%.6f %.6f %d\n",radians_to_degrees(latitude),radians_to_degrees(longitude),count);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for super-nodes and super-segments.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSuper(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_super);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node as a super-node (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_super(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segment;

 if(!IsSuperNode(nodep))
    return;

 printf("%.6f %.6f n\n",radians_to_degrees(latitude),radians_to_degrees(longitude));

 segment=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsSuperSegment(segment))
      {
       index_t othernode=OtherNode(segment,node);
       double lat,lon;

       GetLatLong(OSMNodes,othernode,&lat,&lon);

       if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
          printf("%.6f %.6f s\n",radians_to_degrees(lat),radians_to_degrees(lon));
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for one-way segments.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputOneway(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_oneway);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and all connected one-way segments (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_oneway(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segment;

 segment=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segment))
      {
       index_t othernode=OtherNode(segment,node);

       if(node>othernode)
         {
          double lat,lon;

          GetLatLong(OSMNodes,othernode,&lat,&lon);

          if(IsOnewayFrom(segment,node))
             printf("%.6f %.6f %.6f %.6f\n",radians_to_degrees(latitude),radians_to_degrees(longitude),radians_to_degrees(lat),radians_to_degrees(lon));
          else if(IsOnewayFrom(segment,othernode))
             printf("%.6f %.6f %.6f %.6f\n",radians_to_degrees(lat),radians_to_degrees(lon),radians_to_degrees(latitude),radians_to_degrees(longitude));
         }
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for turn restrictions.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputTurnRestrictions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_turnrestriction);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node as the 'via' node for a turn restriction (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_turnrestriction(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 index_t turnrelation=NO_RELATION;

 if(!IsTurnRestrictedNode(nodep))
    return;

 turnrelation=FindFirstTurnRelation1(OSMRelations,node);

 do
   {
    TurnRelation *relation;
    Segment *from_segment,*to_segment;
    index_t from_node,to_node;
    double from_lat,from_lon,to_lat,to_lon;

    relation=LookupTurnRelation(OSMRelations,turnrelation,1);

    from_segment=LookupSegment(OSMSegments,relation->from,1);
    to_segment  =LookupSegment(OSMSegments,relation->to  ,2);

    from_node=OtherNode(from_segment,node);
    to_node=OtherNode(to_segment,node);

    GetLatLong(OSMNodes,from_node,&from_lat,&from_lon);
    GetLatLong(OSMNodes,to_node,&to_lat,&to_lon);

    printf("%.6f %.6f %.6f %.6f %.6f %.6f\n",radians_to_degrees(from_lat),radians_to_degrees(from_lon),
                                             radians_to_degrees(latitude),radians_to_degrees(longitude),
                                             radians_to_degrees(to_lat),radians_to_degrees(to_lon));

    turnrelation=FindNextTurnRelation1(OSMRelations,turnrelation);
   }
 while(turnrelation!=NO_RELATION);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for speed limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSpeedLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=SPEED_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for weight limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=WEIGHT_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for height limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputHeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=HEIGHT_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for width limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWidthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=WIDTH_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for length limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputLengthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=LENGTH_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node as a speed, weight, height, length, width limit (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_limits(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segment,segments[MAX_SEG_PER_NODE];
 int limits[MAX_SEG_PER_NODE];
 int count=0;
 int i,j,same=0;

 segment=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segment) && count<MAX_SEG_PER_NODE)
      {
       Way *way=LookupWay(OSMWays,segment->way,1);

       segments[count]=*segment;

       switch(limit_type)
         {
         case SPEED_LIMIT:  limits[count]=way->speed;  break;
         case WEIGHT_LIMIT: limits[count]=way->weight; break;
         case HEIGHT_LIMIT: limits[count]=way->height; break;
         case WIDTH_LIMIT:  limits[count]=way->width;  break;
         case LENGTH_LIMIT: limits[count]=way->length; break;
         }

       if(limits[count] || HIGHWAY(way->type)<Way_Track)
          count++;
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);

 /* Nodes with only one limit are not interesting */

 if(count==1)
    return;

 /* Nodes with all segments the same limit are not interesting */

 same=0;
 for(j=0;j<count;j++)
    if(limits[0]==limits[j])
       same++;

 if(same==count)
    return;

 /* Display the interesting limits */

 printf("%.6f %.6f\n",radians_to_degrees(latitude),radians_to_degrees(longitude));

 for(i=0;i<count;i++)
   {
    same=0;
    for(j=0;j<count;j++)
       if(limits[i]==limits[j])
          same++;

    if(count==2 || same!=(count-1))
      {
       double lat,lon;

       GetLatLong(OSMNodes,OtherNode(&segments[i],node),&lat,&lon);

       switch(limit_type)
         {
         case SPEED_LIMIT:
          printf("%.6f %.6f %d\n",radians_to_degrees(lat),radians_to_degrees(lon),speed_to_kph(limits[i]));
          break;
         case WEIGHT_LIMIT:
          printf("%.6f %.6f %.1f\n",radians_to_degrees(lat),radians_to_degrees(lon),weight_to_tonnes(limits[i]));
          break;
         case HEIGHT_LIMIT:
          printf("%.6f %.6f %.1f\n",radians_to_degrees(lat),radians_to_degrees(lon),height_to_metres(limits[i]));
          break;
         case WIDTH_LIMIT:
          printf("%.6f %.6f %.1f\n",radians_to_degrees(lat),radians_to_degrees(lon),width_to_metres(limits[i]));
          break;
         case LENGTH_LIMIT:
          printf("%.6f %.6f %.1f\n",radians_to_degrees(lat),radians_to_degrees(lon),length_to_metres(limits[i]));
          break;
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  A function to iterate through all nodes and call a callback function for each one.

  Nodes *nodes The set of nodes to use.

  callback_t callback The callback function for each node.
  ++++++++++++++++++++++++++++++++++++++*/

static void find_all_nodes(Nodes *nodes,callback_t callback)
{
 ll_bin_t latminbin=latlong_to_bin(radians_to_latlong(LatMin))-nodes->file.latzero;
 ll_bin_t latmaxbin=latlong_to_bin(radians_to_latlong(LatMax))-nodes->file.latzero;
 ll_bin_t lonminbin=latlong_to_bin(radians_to_latlong(LonMin))-nodes->file.lonzero;
 ll_bin_t lonmaxbin=latlong_to_bin(radians_to_latlong(LonMax))-nodes->file.lonzero;
 ll_bin_t latb,lonb;
 index_t i,index1,index2;

 /* Loop through all of the nodes. */

 for(latb=latminbin;latb<=latmaxbin;latb++)
    for(lonb=lonminbin;lonb<=lonmaxbin;lonb++)
      {
       ll_bin2_t llbin=lonb*nodes->file.latbins+latb;

       if(llbin<0 || llbin>(nodes->file.latbins*nodes->file.lonbins))
          continue;

       index1=LookupNodeOffset(nodes,llbin);
       index2=LookupNodeOffset(nodes,llbin+1);

       for(i=index1;i<index2;i++)
         {
          Node *node=LookupNode(nodes,i,1);

          double lat=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(node->latoffset));
          double lon=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(node->lonoffset));

          if(lat>LatMin && lat<LatMax && lon>LonMin && lon<LonMax)
             (*callback)(i,lat,lon);
         }
      }
}
