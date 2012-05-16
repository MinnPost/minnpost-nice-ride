/***************************************
 Fake node and segment generation.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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


#include "types.h"
#include "nodes.h"
#include "segments.h"

#include "fakes.h"


/*+ The minimum distance along a segment from a node to insert a fake node. (in km). +*/
#define MINSEGMENT 0.005


/*+ A set of fake segments to allow start/finish in the middle of a segment. +*/
static Segment fake_segments[4*NWAYPOINTS+1];

/*+ A set of pointers to the real segments underlying the fake segments. +*/
static index_t real_segments[4*NWAYPOINTS+1];

/*+ A set of fake node latitudes and longitudes. +*/
static double fake_lon[NWAYPOINTS+1],fake_lat[NWAYPOINTS+1];

/*+ The previous waypoint. +*/
static int prevpoint=0;


/*++++++++++++++++++++++++++++++++++++++
  Create a pair of fake segments corresponding to the given segment split in two
  (and will create an extra two fake segments if adjacent waypoints are on the
  same segment).

  index_t CreateFakes Returns the fake node index (or a real one in special cases).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  int point Which of the waypoints this is.

  Segment *segment The segment to split.

  index_t node1 The first node at the end of this segment.

  index_t node2 The second node at the end of this segment.

  distance_t dist1 The distance to the first node.

  distance_t dist2 The distance to the second node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t CreateFakes(Nodes *nodes,Segments *segments,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2)
{
 index_t fakenode;
 double lat1,lon1,lat2,lon2;

 /* Initialise the segments to fake values */

 fake_segments[4*point-4].node1=NO_NODE;
 fake_segments[4*point-4].node2=NO_NODE;

 fake_segments[4*point-3].node1=NO_NODE;
 fake_segments[4*point-3].node2=NO_NODE;

 fake_segments[4*point-2].node1=NO_NODE;
 fake_segments[4*point-2].node2=NO_NODE;

 fake_segments[4*point-1].node1=NO_NODE;
 fake_segments[4*point-1].node2=NO_NODE;

 /* Check if we are actually close enough to an existing node */

 if(dist1<km_to_distance(MINSEGMENT) && dist2>km_to_distance(MINSEGMENT))
   {
    prevpoint=point;
    return(node1);
   }

 if(dist2<km_to_distance(MINSEGMENT) && dist1>km_to_distance(MINSEGMENT))
   {
    prevpoint=point;
    return(node2);
   }

 if(dist1<km_to_distance(MINSEGMENT) && dist2<km_to_distance(MINSEGMENT))
   {
    prevpoint=point;

    if(dist1<dist2)
       return(node1);
    else
       return(node2);
   }

 /* Create the fake node */

 fakenode=NODE_FAKE+point;

 GetLatLong(nodes,node1,&lat1,&lon1);
 GetLatLong(nodes,node2,&lat2,&lon2);

 if(lat1>3 && lat2<-3)
    lat2+=2*M_PI;
 else if(lat1<-3 && lat2>3)
    lat1+=2*M_PI;

 fake_lat[point]=lat1+(lat2-lat1)*(double)dist1/(double)(dist1+dist2);
 fake_lon[point]=lon1+(lon2-lon1)*(double)dist1/(double)(dist1+dist2);

 if(fake_lat[point]>M_PI) fake_lat[point]-=2*M_PI;

 /*
  *    node1  fakenode                         node2
  *      #----------*----------------------------#     real_segments[4*point-{4,3}]
  *   
  *      #----------*                                  fake_segments[4*point-4]
  *                 *----------------------------#     fake_segments[4*point-3]
  *   
  *   
  *    node1  fakenode[prevpoint]              node2
  *      #----------*------------------%---------#     real_segments[4*prevpoint-{4,3,1}], real_segments[4*point-{4,3,2}]
  *                              fakenode[point]
  *      #----------*                                  fake_segments[4*prevpoint-4]
  *                 *----------------------------#     fake_segments[4*prevpoint-3]
  *                 *------------------%               fake_segments[4*prevpoint-1]
  *      #-----------------------------%               fake_segments[4*point-4]
  *                                    %---------#     fake_segments[4*point-3]
  *                 *------------------%               fake_segments[4*point-2]
  */

 /* Create the first fake segment */

 fake_segments[4*point-4]=*segment;

 fake_segments[4*point-4].node2=fakenode;

 fake_segments[4*point-4].distance=DISTANCE(dist1)|DISTFLAG(segment->distance);

 real_segments[4*point-4]=IndexSegment(segments,segment);

 /* Create the second fake segment */

 fake_segments[4*point-3]=*segment;

 fake_segments[4*point-3].node1=fakenode;

 fake_segments[4*point-3].distance=DISTANCE(dist2)|DISTFLAG(segment->distance);

 real_segments[4*point-3]=IndexSegment(segments,segment);

 /* Create a third fake segment to join adjacent points if both are fake and on the same real segment */

 if(prevpoint>0 && fake_segments[4*prevpoint-4].node1==node1 && fake_segments[4*prevpoint-3].node2==node2)
   {
    if(DISTANCE(dist1)>DISTANCE(fake_segments[4*prevpoint-4].distance)) /* point is further from node1 than prevpoint */
      {
       fake_segments[4*point-2]=fake_segments[4*prevpoint-3];

       fake_segments[4*point-2].node2=fakenode;

       fake_segments[4*point-2].distance=(DISTANCE(dist1)-DISTANCE(fake_segments[4*prevpoint-4].distance))|DISTFLAG(segment->distance);
      }
    else
      {
       fake_segments[4*point-2]=fake_segments[4*prevpoint-4];

       fake_segments[4*point-2].node1=fakenode;

       fake_segments[4*point-2].distance=(DISTANCE(fake_segments[4*prevpoint-4].distance)-DISTANCE(dist1))|DISTFLAG(segment->distance);
      }

    real_segments[4*point-2]=IndexSegment(segments,segment);

    fake_segments[4*prevpoint-1]=fake_segments[4*point-2];

    real_segments[4*prevpoint-1]=real_segments[4*point-2];
   }

 /* Return the fake node */

 prevpoint=point;

 return(fakenode);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup the latitude and longitude of a fake node.

  index_t fakenode The fake node to lookup.

  double *latitude Returns the latitude

  double *longitude Returns the longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetFakeLatLong(index_t fakenode, double *latitude,double *longitude)
{
 index_t whichnode=fakenode-NODE_FAKE;

 *latitude =fake_lat[whichnode];
 *longitude=fake_lon[whichnode];
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the first fake segment associated to a fake node.

  Segment *FirstFakeSegment Returns a pointer to the first fake segment.

  index_t fakenode The fake node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FirstFakeSegment(index_t fakenode)
{
 index_t whichnode=fakenode-NODE_FAKE;

 return(&fake_segments[4*whichnode-4]);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the next fake segment associated to a fake node.

  Segment *NextFakeSegment Returns a pointer to the next fake segment.

  Segment *fakesegment The first fake segment.

  index_t fakenode The node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextFakeSegment(Segment *fakesegment,index_t fakenode)
{
 index_t whichnode=fakenode-NODE_FAKE;

 if(fakesegment==&fake_segments[4*whichnode-4])
    return(&fake_segments[4*whichnode-3]);

 if(fakesegment==&fake_segments[4*whichnode-3] && fake_segments[4*whichnode-2].node1!=NO_NODE)
    return(&fake_segments[4*whichnode-2]);

 if(fakesegment==&fake_segments[4*whichnode-3] && fake_segments[4*whichnode-1].node1!=NO_NODE)
    return(&fake_segments[4*whichnode-1]);

 if(fakesegment==&fake_segments[4*whichnode-2] && fake_segments[4*whichnode-1].node1!=NO_NODE)
    return(&fake_segments[4*whichnode-1]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the fake segment between a real node and a fake node.

  Segment *ExtraFakeSegment Returns a segment between the two specified nodes if it exists.

  index_t realnode The real node.

  index_t fakenode The fake node.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *ExtraFakeSegment(index_t realnode,index_t fakenode)
{
 index_t whichnode=fakenode-NODE_FAKE;

 if(fake_segments[4*whichnode-4].node1==realnode || fake_segments[4*whichnode-4].node2==realnode)
    return(&fake_segments[4*whichnode-4]);

 if(fake_segments[4*whichnode-3].node1==realnode || fake_segments[4*whichnode-3].node2==realnode)
    return(&fake_segments[4*whichnode-3]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a fake segment given its index.

  Segment *LookupFakeSegment Returns a pointer to the fake segment.

  index_t fakesegment The index of the fake segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *LookupFakeSegment(index_t fakesegment)
{
 index_t whichsegment=fakesegment-SEGMENT_FAKE;

 return(&fake_segments[whichsegment]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the fake index of a fake segment.

  index_t IndexFakeSegment Returns the fake segment.

  Segment *fakesegment The fake segment to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexFakeSegment(Segment *fakesegment)
{
 index_t whichsegment=fakesegment-&fake_segments[0];

 return(whichsegment+SEGMENT_FAKE);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the real segment underlying a fake segment.

  index_t IndexRealSegment Returns the index of the real segment.

  index_t fakesegment The index of the fake segment.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexRealSegment(index_t fakesegment)
{
 index_t whichsegment=fakesegment-SEGMENT_FAKE;

 return(real_segments[whichsegment]);
}


/*++++++++++++++++++++++++++++++++++++++
  Determine if a route between two fake segments is valid or a U-turn.

  int IsFakeUTurn Returns true for a U-turn.

  index_t fakesegment1 The first fake segment.

  index_t fakesegment2 The second fake segment.
  ++++++++++++++++++++++++++++++++++++++*/

int IsFakeUTurn(index_t fakesegment1,index_t fakesegment2)
{
 index_t whichsegment1=fakesegment1-SEGMENT_FAKE;
 index_t whichsegment2=fakesegment2-SEGMENT_FAKE;

 if(fake_segments[whichsegment1].node1==fake_segments[whichsegment2].node1)
    return(1);

 if(fake_segments[whichsegment1].node2==fake_segments[whichsegment2].node2)
    return(1);

 return(0);
}
