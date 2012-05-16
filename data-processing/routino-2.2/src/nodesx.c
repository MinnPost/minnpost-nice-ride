/***************************************
 Extented Node data type functions.

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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "nodes.h"

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/*+ A temporary file-local variable for use by the sort functions. +*/
static NodesX *sortnodesx;

/* Functions */

static int sort_by_id(NodeX *a,NodeX *b);
static int deduplicate_and_index_by_id(NodeX *nodex,index_t index);

static int sort_by_lat_long(NodeX *a,NodeX *b);
static int delete_pruned_and_index_by_lat_long(NodeX *nodex,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list (create a new file or open an existing one).

  NodesX *NewNodeList Returns a pointer to the node list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(int append)
{
 NodesX *nodesx;

 nodesx=(NodesX*)calloc(1,sizeof(NodesX));

 assert(nodesx); /* Check calloc() worked */

 nodesx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(nodesx->filename,"%s/nodesx.input.tmp",option_tmpdirname);
 else
    sprintf(nodesx->filename,"%s/nodesx.%p.tmp",option_tmpdirname,(void*)nodesx);

 if(append)
   {
    off_t size;

    nodesx->fd=OpenFileAppend(nodesx->filename);

    size=SizeFile(nodesx->filename);

    nodesx->number=size/sizeof(NodeX);
   }
 else
    nodesx->fd=OpenFileNew(nodesx->filename);

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a node list.

  NodesX *nodesx The set of nodes to be freed.

  int keep Set to 1 if the file is to be kept (for appending later).
  ++++++++++++++++++++++++++++++++++++++*/

void FreeNodeList(NodesX *nodesx,int keep)
{
 if(!keep)
    DeleteFile(nodesx->filename);

 free(nodesx->filename);

 if(nodesx->idata)
    free(nodesx->idata);

 if(nodesx->gdata)
    free(nodesx->gdata);

 if(nodesx->super)
    free(nodesx->super);

 free(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single node to an unsorted node list.

  NodesX *nodesx The set of nodes to modify.

  node_t id The node identifier from the original OSM data.

  double latitude The latitude of the node.

  double longitude The longitude of the node.

  transports_t allow The allowed traffic types through the node.

  uint16_t flags The flags to set for this node.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendNode(NodesX *nodesx,node_t id,double latitude,double longitude,transports_t allow,uint16_t flags)
{
 NodeX nodex;

 nodex.id=id;
 nodex.latitude =radians_to_latlong(latitude);
 nodex.longitude=radians_to_latlong(longitude);
 nodex.allow=allow;
 nodex.flags=flags;

 WriteFile(nodesx->fd,&nodex,sizeof(NodeX));

 nodesx->number++;

 assert(nodesx->number<NODE_FAKE); /* NODE_FAKE marks the high-water mark for real nodes. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesX *nodesx The set of nodes to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX *nodesx)
{
 int fd;
 index_t xnumber;

 /* Print the start message */

 printf_first("Sorting Nodes");

 /* Close the file (finished appending) */

 nodesx->fd=CloseFile(nodesx->fd);

 /* Re-open the file read-only and a new file writeable */

 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Allocate the array of indexes */

 nodesx->idata=(node_t*)malloc(nodesx->number*sizeof(node_t));

 assert(nodesx->idata); /* Check malloc() worked */

 /* Sort by node indexes */

 xnumber=nodesx->number;
 nodesx->number=0;

 sortnodesx=nodesx;

 nodesx->number=filesort_fixed(nodesx->fd,fd,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))deduplicate_and_index_by_id);

 /* Close the files */

 nodesx->fd=CloseFile(nodesx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Nodes: Nodes=%"Pindex_t" Duplicates=%"Pindex_t,xnumber,xnumber-nodesx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  NodeX *a The first extended node.

  NodeX *b The second extended node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(NodeX *a,NodeX *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the index of identifiers and discard duplicate nodes.

  int deduplicate_and_index_by_id Return 1 if the value is to be kept, otherwise 0.

  NodeX *nodex The extended node.

  index_t index The index of this node in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int deduplicate_and_index_by_id(NodeX *nodex,index_t index)
{
 if(index==0 || sortnodesx->idata[index-1]!=nodex->id)
   {
    sortnodesx->idata[index]=nodex->id;

    return(1);
   }
 else
   {
    logerror("Node %"Pnode_t" is duplicated\n",nodex->id);

    return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list geographically.

  NodesX *nodesx The set of nodes to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeListGeographically(NodesX *nodesx)
{
 int fd;
 index_t kept;

 /* Print the start message */

 printf_first("Sorting Nodes Geographically (Deleting Pruned)");

 /* Allocate the memory for the geographical index array */

 nodesx->gdata=(index_t*)malloc(nodesx->number*sizeof(index_t));

 assert(nodesx->gdata); /* Check malloc() worked */

 /* Re-open the file read-only and a new file writeable */

 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Sort geographically */

 sortnodesx=nodesx;

 kept=filesort_fixed(nodesx->fd,fd,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_lat_long,(int (*)(void*,index_t))delete_pruned_and_index_by_lat_long);

 /* Close the files */

 nodesx->fd=CloseFile(nodesx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Nodes Geographically: Nodes=%"Pindex_t" Deleted=%"Pindex_t,kept,nodesx->number-kept);
 nodesx->number=kept;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into latitude and longitude order (first by longitude bin
  number, then by latitude bin number and then by exact longitude and then by
  exact latitude).

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  NodeX *a The first extended node.

  NodeX *b The second extended node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX *a,NodeX *b)
{
 int a_pruned=IsPrunedNodeX(a);
 int b_pruned=IsPrunedNodeX(b);

 if(a_pruned && b_pruned)
    return(0);
 else if(a_pruned)
    return(1);
 else if(b_pruned)
    return(-1);
 else
   {
    ll_bin_t a_lon=latlong_to_bin(a->longitude);
    ll_bin_t b_lon=latlong_to_bin(b->longitude);

    if(a_lon<b_lon)
       return(-1);
    else if(a_lon>b_lon)
       return(1);
    else
      {
       ll_bin_t a_lat=latlong_to_bin(a->latitude);
       ll_bin_t b_lat=latlong_to_bin(b->latitude);

       if(a_lat<b_lat)
          return(-1);
       else if(a_lat>b_lat)
          return(1);
       else
         {
          if(a->longitude<b->longitude)
             return(-1);
          else if(a->longitude>b->longitude)
             return(1);
          else
            {
             if(a->latitude<b->latitude)
                return(-1);
             else if(a->latitude>b->latitude)
                return(1);
            }

          return(0);
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Delete the pruned nodes and create the index between the sorted and unsorted nodes.

  int delete_pruned_and_index_by_lat_long Return 1 if the value is to be kept, otherwise 0.

  NodeX *nodex The extended node.

  index_t index The index of this node in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int delete_pruned_and_index_by_lat_long(NodeX *nodex,index_t index)
{
 if(IsPrunedNodeX(nodex))
    return(0);

 /* Create the index from the previous sort to the current one */

 sortnodesx->gdata[nodex->id]=index;

 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node index.

  index_t IndexNodeX Returns the index of the extended node with the specified id.

  NodesX *nodesx The set of nodes to use.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexNodeX(NodesX *nodesx,node_t id)
{
 index_t start=0;
 index_t end=nodesx->number-1;
 index_t mid;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                        /* There are no nodes */
    return(NO_NODE);
 else if(id<nodesx->idata[start])     /* Check key is not before start */
    return(NO_NODE);
 else if(id>nodesx->idata[end])       /* Check key is not after end */
    return(NO_NODE);
 else
   {
    do
      {
       mid=(start+end)/2;             /* Choose mid point */

       if(nodesx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesx->idata[mid]>id) /* Mid point is too high */
          end=mid?(mid-1):mid;
       else                           /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(nodesx->idata[start]==id)      /* Start is correct */
       return(start);

    if(nodesx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_NODE);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove any nodes that are not part of a highway.

  NodesX *nodesx The set of nodes to modify.

  SegmentsX *segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 NodeX nodex;
 index_t total=0,highway=0,nothighway=0;
 ll_bin_t lat_min_bin,lat_max_bin,lon_min_bin,lon_max_bin;
 latlong_t lat_min,lat_max,lon_min,lon_max;
 int fd;

 /* Print the start message */

 printf_first("Checking Nodes: Nodes=0");

 /* While we are here we can work out the range of data */

 lat_min=radians_to_latlong( 2);
 lat_max=radians_to_latlong(-2);
 lon_min=radians_to_latlong( 4);
 lon_max=radians_to_latlong(-4);

 /* Re-open the file read-only and a new file writeable */

 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(nodesx->fd,&nodex,sizeof(NodeX)))
   {
    if(!IsBitSet(segmentsx->usednode,total))
       nothighway++;
    else
      {
       nodex.id=highway;

       WriteFile(fd,&nodex,sizeof(NodeX));

       nodesx->idata[highway]=nodesx->idata[total];
       highway++;

       if(nodex.latitude<lat_min)
          lat_min=nodex.latitude;
       if(nodex.latitude>lat_max)
          lat_max=nodex.latitude;
       if(nodex.longitude<lon_min)
          lon_min=nodex.longitude;
       if(nodex.longitude>lon_max)
          lon_max=nodex.longitude;
      }

    total++;

    if(!(total%10000))
       printf_middle("Checking Nodes: Nodes=%"Pindex_t" Highway=%"Pindex_t" not-Highway=%"Pindex_t,total,highway,nothighway);
   }

 nodesx->number=highway;

 /* Close the files */

 nodesx->fd=CloseFile(nodesx->fd);
 CloseFile(fd);

 /* Work out the number of bins */

 lat_min_bin=latlong_to_bin(lat_min);
 lon_min_bin=latlong_to_bin(lon_min);
 lat_max_bin=latlong_to_bin(lat_max);
 lon_max_bin=latlong_to_bin(lon_max);

 nodesx->latzero=lat_min_bin;
 nodesx->lonzero=lon_min_bin;

 nodesx->latbins=(lat_max_bin-lat_min_bin)+1;
 nodesx->lonbins=(lon_max_bin-lon_min_bin)+1;

 /* Free the now-unneeded index */

 free(segmentsx->usednode);
 segmentsx->usednode=NULL;

 /* Print the final message */

 printf_last("Checked Nodes: Nodes=%"Pindex_t" Highway=%"Pindex_t" not-Highway=%"Pindex_t,total,highway,nothighway);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert the super-node flag and the first segment indexes after geographical sorting.

  NodesX *nodesx The set of nodes to modify.

  SegmentsX *segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 index_t i;
 int fd;

 /* Print the start message */

 printf_first("Updating Super Nodes: Nodes=0");

 /* Re-open the file read-only and a new file writeable */

 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Modify the on-disk image */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX nodex;

    ReadFile(nodesx->fd,&nodex,sizeof(NodeX));

    if(IsBitSet(nodesx->super,nodex.id))
       nodex.flags|=NODE_SUPER;

    nodex.id=segmentsx->firstnode[nodesx->gdata[nodex.id]];

    WriteFile(fd,&nodex,sizeof(NodeX));

    if(!((i+1)%10000))
       printf_middle("Updating Super Nodes: Nodes=%"Pindex_t,i+1);
   }

 /* Close the files */

 nodesx->fd=CloseFile(nodesx->fd);
 CloseFile(fd);

 /* Free the memory */

 free(nodesx->super);
 nodesx->super=NULL;

 /* Print the final message */

 printf_last("Updated Super Nodes: Nodes=%"Pindex_t,nodesx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the final node list database to a file.

  NodesX *nodesx The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesX *nodesx,const char *filename)
{
 index_t i;
 int fd;
 NodesFile nodesfile={0};
 index_t super_number=0;
 ll_bin2_t latlonbin=0,maxlatlonbins;
 index_t *offsets;

 /* Print the start message */

 printf_first("Writing Nodes: Nodes=0");

 /* Allocate the memory for the geographical offsets array */

 offsets=(index_t*)malloc((nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 assert(offsets); /* Check malloc() worked */

 latlonbin=0;

 /* Re-open the file */

 nodesx->fd=ReOpenFile(nodesx->filename);

 /* Write out the nodes data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(NodesFile)+(nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 for(i=0;i<nodesx->number;i++)
   {
    NodeX nodex;
    Node node={0};
    ll_bin_t latbin,lonbin;
    ll_bin2_t llbin;

    ReadFile(nodesx->fd,&nodex,sizeof(NodeX));

    /* Create the Node */

    node.latoffset=latlong_to_off(nodex.latitude);
    node.lonoffset=latlong_to_off(nodex.longitude);
    node.firstseg=nodex.id;
    node.allow=nodex.allow;
    node.flags=nodex.flags;

    if(node.flags&NODE_SUPER)
       super_number++;

    /* Work out the offsets */

    latbin=latlong_to_bin(nodex.latitude )-nodesx->latzero;
    lonbin=latlong_to_bin(nodex.longitude)-nodesx->lonzero;
    llbin=lonbin*nodesx->latbins+latbin;

    for(;latlonbin<=llbin;latlonbin++)
       offsets[latlonbin]=i;

    /* Write the data */

    WriteFile(fd,&node,sizeof(Node));

    if(!((i+1)%10000))
       printf_middle("Writing Nodes: Nodes=%"Pindex_t,i+1);
   }

 /* Close the file */

 nodesx->fd=CloseFile(nodesx->fd);

 /* Finish off the offset indexing and write them out */

 maxlatlonbins=nodesx->latbins*nodesx->lonbins;

 for(;latlonbin<=maxlatlonbins;latlonbin++)
    offsets[latlonbin]=nodesx->number;

 SeekFile(fd,sizeof(NodesFile));
 WriteFile(fd,offsets,(nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 free(offsets);

 /* Write out the header structure */

 nodesfile.number=nodesx->number;
 nodesfile.snumber=super_number;

 nodesfile.latbins=nodesx->latbins;
 nodesfile.lonbins=nodesx->lonbins;

 nodesfile.latzero=nodesx->latzero;
 nodesfile.lonzero=nodesx->lonzero;

 SeekFile(fd,0);
 WriteFile(fd,&nodesfile,sizeof(NodesFile));

 CloseFile(fd);

 /* Print the final message */

 printf_last("Wrote Nodes: Nodes=%"Pindex_t,nodesx->number);
}
