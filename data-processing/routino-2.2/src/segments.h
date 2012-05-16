/***************************************
 $Header: /home/amb/CVS/routino/src/segments.h,v 1.38 2010-12-21 17:18:41 amb Exp $

 A header file for the segments.

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


#ifndef SEGMENTS_H
#define SEGMENTS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "files.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single segment. +*/
struct _Segment
{
 index_t    node1;              /*+ The index of the starting node. +*/
 index_t    node2;              /*+ The index of the finishing node. +*/

 index_t    next2;              /*+ The index of the next segment sharing node2. +*/

 index_t    way;                /*+ The index of the way associated with the segment. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _SegmentsFile
{
 index_t   number;              /*+ The number of segments in total. +*/
 index_t   snumber;             /*+ The number of super-segments. +*/
 index_t   nnumber;             /*+ The number of normal segments. +*/
}
 SegmentsFile;


/*+ A structure containing a set of segments (and pointers to mmap file). +*/
struct _Segments
{
 SegmentsFile file;             /*+ The header data from the file. +*/

#if !SLIM

 void        *data;             /*+ The memory mapped data. +*/

 Segment     *segments;         /*+ An array of segments. +*/

#else

 int          fd;               /*+ The file descriptor for the file. +*/

 Segment      cached[3];        /*+ Three cached segments read from the file in slim mode. +*/
 index_t      incache[3];       /*+ The indexes of the cached segments. +*/

#endif
};


/* Functions in segments.c */

Segments *LoadSegmentList(const char *filename);

index_t FindClosestSegmentHeading(Nodes *nodes,Segments *segments,Ways *ways,index_t node1,double heading,Profile *profile);

distance_t Distance(double lat1,double lon1,double lat2,double lon2);

duration_t Duration(Segment *segment,Way *way,Profile *profile);

double TurnAngle(Nodes *nodes,Segment *segment1,Segment *segment2,index_t node);
double BearingAngle(Nodes *nodes,Segment *segment,index_t node);


static inline Segment *NextSegment(Segments *segments,Segment *segment,index_t node);


/* Macros and inline functions */

/*+ Return true if this is a normal segment. +*/
#define IsNormalSegment(xxx)   (((xxx)->distance)&SEGMENT_NORMAL)

/*+ Return true if this is a super-segment. +*/
#define IsSuperSegment(xxx)    (((xxx)->distance)&SEGMENT_SUPER)

/*+ Return true if the segment is oneway. +*/
#define IsOneway(xxx)          ((xxx)->distance&(ONEWAY_2TO1|ONEWAY_1TO2))

/*+ Return true if the segment is oneway towards the specified node. +*/
#define IsOnewayTo(xxx,yyy)    ((xxx)->node1==(yyy)?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return true if the segment is oneway from the specified node. +*/
#define IsOnewayFrom(xxx,yyy)  ((xxx)->node2==(yyy)?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return the other node in the segment that is not the specified node. +*/
#define OtherNode(xxx,yyy)     ((xxx)->node1==(yyy)?(xxx)->node2:(xxx)->node1)

#if !SLIM

/*+ Return a segment pointer given a set of segments and an index. +*/
#define LookupSegment(xxx,yyy,ppp) (&(xxx)->segments[yyy])

/*+ Return a segment index given a set of segments and a pointer. +*/
#define IndexSegment(xxx,yyy)      (index_t)((yyy)-&(xxx)->segments[0])


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment.

  Segments *segments The set of segments to use.

  Segment *segment The current segment.

  index_t node The wanted node.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Segment *NextSegment(Segments *segments,Segment *segment,index_t node)
{
 if(segment->node1==node)
   {
    segment++;

    if(IndexSegment(segments,segment)>=segments->file.number || segment->node1!=node)
       return(NULL);
    else
       return(segment);
   }
 else
   {
    if(segment->next2==NO_SEGMENT)
       return(NULL);
    else
       return(LookupSegment(segments,segment->next2,1));
   }
}

#else

static Segment *LookupSegment(Segments *segments,index_t index,int position);

static index_t IndexSegment(Segments *segments,Segment *segment);


/*++++++++++++++++++++++++++++++++++++++
  Find the Segment information for a particular segment.

  Segment *LookupSegment Returns a pointer to the cached segment information.

  Segments *segments The set of segments to use.

  index_t index The index of the segment.

  int position The position in the cache to store the value.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Segment *LookupSegment(Segments *segments,index_t index,int position)
{
 if(segments->incache[position-1]!=index)
   {
    SeekReadFile(segments->fd,&segments->cached[position-1],sizeof(Segment),sizeof(SegmentsFile)+(off_t)index*sizeof(Segment));

    segments->incache[position-1]=index;
   }

 return(&segments->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the segment index for a particular segment pointer.

  index_t IndexSegment Returns the index of the segment in the list.

  Segments *segments The set of segments to use.

  Segment *segment The segment whose index is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t IndexSegment(Segments *segments,Segment *segment)
{
 int position1=segment-&segments->cached[0];

 return(segments->incache[position1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment.

  Segments *segments The set of segments to use.

  Segment *segment The current segment.

  index_t node The wanted node.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Segment *NextSegment(Segments *segments,Segment *segment,index_t node)
{
 int position=segment-&segments->cached[-1];

 if(segment->node1==node)
   {
    index_t index=IndexSegment(segments,segment);

    index++;

    if(index>=segments->file.number)
       return(NULL);

    segment=LookupSegment(segments,index,position);

    if(segment->node1!=node)
       return(NULL);
    else
       return(segment);
   }
 else
   {
    if(segment->next2==NO_SEGMENT)
       return(NULL);
    else
       return(LookupSegment(segments,segment->next2,position));
   }
}

#endif


#endif /* SEGMENTS_H */
