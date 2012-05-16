/***************************************
 A header file for the extended nodes.

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


#ifndef NODESX_H
#define NODESX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "nodes.h"

#include "typesx.h"

#include "files.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _NodeX
{
 node_t       id;               /*+ The node identifier; initially the OSM value, later the Node index, finally the first segment. +*/

 latlong_t    latitude;         /*+ The node latitude. +*/
 latlong_t    longitude;        /*+ The node longitude. +*/

 transports_t allow;            /*+ The node allowed traffic. +*/

 uint16_t     flags;            /*+ The node flags. +*/
};

/*+ A structure containing a set of nodes (memory format). +*/
struct _NodesX
{
 char     *filename;            /*+ The name of the temporary file. +*/
 int       fd;                  /*+ The file descriptor of the temporary file. +*/

 index_t   number;              /*+ The number of extended nodes still being considered. +*/

#if !SLIM

 NodeX    *data;                /*+ The extended node data (when mapped into memory). +*/

#else

 NodeX     cached[3];           /*+ Three cached extended nodes read from the file in slim mode. +*/
 index_t   incache[3];          /*+ The indexes of the cached extended nodes. +*/

#endif

 node_t   *idata;               /*+ The extended node IDs (sorted by ID). +*/

 index_t  *gdata;               /*+ The final node indexes (sorted geographically). +*/

 BitMask  *super;               /*+ A bit-mask marker for super nodes (same order as sorted nodes). +*/

 index_t   latbins;             /*+ The number of bins containing latitude. +*/
 index_t   lonbins;             /*+ The number of bins containing longitude. +*/

 ll_bin_t  latzero;             /*+ The bin number of the furthest south bin. +*/
 ll_bin_t  lonzero;             /*+ The bin number of the furthest west bin. +*/
};


/* Functions in nodesx.c */

NodesX *NewNodeList(int append);
void FreeNodeList(NodesX *nodesx,int keep);

void SaveNodeList(NodesX *nodesx,const char *filename);

index_t IndexNodeX(NodesX *nodesx,node_t id);

void AppendNode(NodesX *nodesx,node_t id,double latitude,double longitude,transports_t allow,uint16_t flags);

void SortNodeList(NodesX *nodesx);

void SortNodeListGeographically(NodesX *nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void UpdateNodes(NodesX *nodesx,SegmentsX *segmentsx);


/* Macros and inline functions */

/*+ Return true if this is a pruned node. +*/
#define IsPrunedNodeX(xxx)   ((xxx)->latitude==NO_LATLONG)


#if !SLIM

#define LookupNodeX(nodesx,index,position)      &(nodesx)->data[index]
  
#define PutBackNodeX(nodesx,nodex)              /* nop */

#else

static NodeX *LookupNodeX(NodesX *nodesx,index_t index,int position);

static void PutBackNodeX(NodesX *nodesx,NodeX *nodex);


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended node with the specified id from the file on disk.

  NodeX *LookupNodeX Returns a pointer to a cached copy of the extended node.

  NodesX *nodesx The set of nodes to use.

  index_t index The node index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline NodeX *LookupNodeX(NodesX *nodesx,index_t index,int position)
{
 SeekReadFile(nodesx->fd,&nodesx->cached[position-1],sizeof(NodeX),(off_t)index*sizeof(NodeX));

 nodesx->incache[position-1]=index;

 return(&nodesx->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Put back an extended node's data into the file on disk.

  NodesX *nodesx The set of nodes to modify.

  NodeX *nodex The extended node to be put back.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void PutBackNodeX(NodesX *nodesx,NodeX *nodex)
{
 int position1=nodex-&nodesx->cached[0];

 SeekWriteFile(nodesx->fd,&nodesx->cached[position1],sizeof(NodeX),(off_t)nodesx->incache[position1]*sizeof(NodeX));
}

#endif /* SLIM */


#endif /* NODESX_H */
