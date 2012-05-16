/***************************************
 A header file for the extended segments.

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


#ifndef SEGMENTSX_H
#define SEGMENTSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "typesx.h"

#include "files.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _SegmentX
{
 node_t     node1;              /*+ The id of the starting node; initially the OSM value, later the NodeX index. +*/
 node_t     node2;              /*+ The id of the finishing node; initially the OSM value, later the NodeX index. +*/

 index_t    next2;              /*+ The index of the next segment with the same node2. +*/

 way_t      way;                /*+ The id of the way; initially the OSM value, later the WayX index. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing a set of segments (memory format). +*/
struct _SegmentsX
{
 char      *filename;           /*+ The name of the temporary file. +*/
 int        fd;                 /*+ The file descriptor of the temporary file. +*/

 index_t    number;             /*+ The number of extended segments still being considered. +*/

#if !SLIM

 SegmentX  *data;               /*+ The extended segment data (when mapped into memory). +*/

#else

 SegmentX   cached[4];          /*+ Four cached extended segments read from the file in slim mode. +*/
 index_t    incache[4];         /*+ The indexes of the cached extended segments. +*/

#endif

 index_t   *firstnode;          /*+ The first segment index for each node. +*/

 index_t   *next1;              /*+ The index of the next segment with the same node1 (used while pruning). +*/

 BitMask   *usednode;           /*+ A flag to indicate if a node is used (used for removing bad segments). +*/
};


/* Functions in segmentsx.c */


SegmentsX *NewSegmentList(int append);
void FreeSegmentList(SegmentsX *segmentsx,int keep);

void SaveSegmentList(SegmentsX *segmentsx,const char *filename);

SegmentX *FirstSegmentX(SegmentsX *segmentsx,index_t nodeindex,int position);
SegmentX *NextSegmentX(SegmentsX *segmentsx,SegmentX *segmentx,index_t nodeindex);

void AppendSegment(SegmentsX *segmentsx,way_t way,node_t node1,node_t node2,distance_t distance);

void SortSegmentList(SegmentsX *segmentsx,int delete);

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx);

void MeasureSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx);

void DeduplicateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx);

void CreateRealSegments(SegmentsX *segmentsx,WaysX *waysx);

void IndexSegments(SegmentsX *segmentsx,NodesX *nodesx);

void UpdateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx);


/* Macros / inline functions */

/*+ Return true if this is a pruned segment. +*/
#define IsPrunedSegmentX(xxx)   ((xxx)->node1==NO_NODE)


#if !SLIM

#define LookupSegmentX(segmentsx,index,position)         &(segmentsx)->data[index]

#define IndexSegmentX(segmentsx,segmentx)                (index_t)((segmentx)-&(segmentsx)->data[0])

#define PutBackSegmentX(segmentsx,segmentx)              /* nop */
  
#else

static SegmentX *LookupSegmentX(SegmentsX *segmentsx,index_t index,int position);

static index_t IndexSegmentX(SegmentsX *segmentsx,SegmentX *segmentx);

static void PutBackSegmentX(SegmentsX *segmentsx,SegmentX *segmentx);


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended segment with the specified id from the file on disk.

  SegmentX *LookupSegmentX Returns a pointer to a cached copy of the extended segment.

  SegmentsX *segmentsx The set of segments to use.

  index_t index The segment index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline SegmentX *LookupSegmentX(SegmentsX *segmentsx,index_t index,int position)
{
 SeekReadFile(segmentsx->fd,&segmentsx->cached[position-1],sizeof(SegmentX),(off_t)index*sizeof(SegmentX));

 segmentsx->incache[position-1]=index;

 return(&segmentsx->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the extended segment index for a particular extended segment pointer.

  index_t IndexSegmentX Returns the index of the extended segment.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The extended segment whose index is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t IndexSegmentX(SegmentsX *segmentsx,SegmentX *segmentx)
{
 int position1=segmentx-&segmentsx->cached[0];

 return(segmentsx->incache[position1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Put back an extended segment's data into the file on disk.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The extended segment to be put back.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void PutBackSegmentX(SegmentsX *segmentsx,SegmentX *segmentx)
{
 int position1=segmentx-&segmentsx->cached[0];

 SeekWriteFile(segmentsx->fd,&segmentsx->cached[position1],sizeof(SegmentX),(off_t)segmentsx->incache[position1]*sizeof(SegmentX));
}

#endif /* SLIM */


#endif /* SEGMENTSX_H */
