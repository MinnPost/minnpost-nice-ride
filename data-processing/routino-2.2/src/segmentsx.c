/***************************************
 Extended Segment data type functions.

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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "segments.h"
#include "ways.h"

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Global variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/* Local functions */

static int sort_by_id(SegmentX *a,SegmentX *b);
static int delete_pruned(SegmentX *segmentx,index_t index);

static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list (create a new file or open an existing one).

  SegmentsX *NewSegmentList Returns the segment list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(int append)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)calloc(1,sizeof(SegmentsX));

 assert(segmentsx); /* Check calloc() worked */

 segmentsx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(segmentsx->filename,"%s/segmentsx.input.tmp",option_tmpdirname);
 else
    sprintf(segmentsx->filename,"%s/segmentsx.%p.tmp",option_tmpdirname,(void*)segmentsx);

 if(append)
   {
    off_t size;

    segmentsx->fd=OpenFileAppend(segmentsx->filename);

    size=SizeFile(segmentsx->filename);

    segmentsx->number=size/sizeof(SegmentX);
   }
 else
    segmentsx->fd=OpenFileNew(segmentsx->filename);

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The set of segments to be freed.

  int keep Set to 1 if the file is to be kept (for appending later).
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx,int keep)
{
 if(!keep)
    DeleteFile(segmentsx->filename);

 free(segmentsx->filename);

 if(segmentsx->firstnode)
    free(segmentsx->firstnode);

 if(segmentsx->next1)
    free(segmentsx->next1);

 if(segmentsx->usednode)
    free(segmentsx->usednode);

 free(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single segment to an unsorted segment list.

  SegmentsX *segmentsx The set of segments to modify.

  way_t way The way that the segment belongs to.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  distance_t distance The distance between the nodes (or just the flags).
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(SegmentsX *segmentsx,way_t way,node_t node1,node_t node2,distance_t distance)
{
 SegmentX segmentx;

 if(node1>node2)
   {
    node_t temp;

    temp=node1;
    node1=node2;
    node2=temp;

    if(distance&(ONEWAY_2TO1|ONEWAY_1TO2))
       distance^=ONEWAY_2TO1|ONEWAY_1TO2;
   }

 segmentx.node1=node1;
 segmentx.node2=node2;
 segmentx.next2=NO_SEGMENT;
 segmentx.way=way;
 segmentx.distance=distance;

 WriteFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

 segmentsx->number++;

 assert(segmentsx->number<SEGMENT_FAKE); /* SEGMENT_FAKE marks the high-water mark for real segments. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsX *segmentsx The set of segments to sort and modify.

  int delete Set to true if pruned segments are to be deleted.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsX *segmentsx,int delete)
{
 int fd;
 index_t kept;

 /* Print the start message */

 if(delete)
    printf_first("Sorting Segments (Deleting Pruned)");
 else
    printf_first("Sorting Segments");

 /* Close the file (finished appending) */

 if(segmentsx->fd!=-1)
    segmentsx->fd=CloseFile(segmentsx->fd);

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Sort by node indexes */

 if(delete)
    kept=filesort_fixed(segmentsx->fd,fd,sizeof(SegmentX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))delete_pruned);
 else
    filesort_fixed(segmentsx->fd,fd,sizeof(SegmentX),(int (*)(const void*,const void*))sort_by_id,NULL);

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Print the final message */

 if(delete)
   {
    printf_last("Sorted Segments: Segments=%"Pindex_t" Deleted=%"Pindex_t,kept,segmentsx->number-kept);
    segmentsx->number=kept;
   }
 else
    printf_last("Sorted Segments: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order, first by node1 then by node2, finally by distance.

  int sort_by_id Returns the comparison of the node fields.

  SegmentX *a The first segment.

  SegmentX *b The second segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SegmentX *a,SegmentX *b)
{
 node_t a_id1=a->node1;
 node_t b_id1=b->node1;

 if(a_id1<b_id1)
    return(-1);
 else if(a_id1>b_id1)
    return(1);
 else /* if(a_id1==b_id1) */
   {
    node_t a_id2=a->node2;
    node_t b_id2=b->node2;

    if(a_id2<b_id2)
       return(-1);
    else if(a_id2>b_id2)
       return(1);
    else
      {
       distance_t a_distance=DISTANCE(a->distance);
       distance_t b_distance=DISTANCE(b->distance);

       if(a_distance<b_distance)
          return(-1);
       else if(a_distance>b_distance)
          return(1);
       else
          return(0);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Delete the pruned segments.

  int delete_pruned Return 1 if the value is to be kept, otherwise 0.

  SegmentX *segmentx The extended segment.

  index_t index The index of this segment in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int delete_pruned(SegmentX *segmentx,index_t index)
{
 if(IsPrunedSegmentX(segmentx))
    return(0);

 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first extended segment with a particular starting node index.
 
  SegmentX *FirstSegmentX Returns a pointer to the first extended segment with the specified id.

  SegmentsX *segmentsx The set of segments to use.

  index_t nodeindex The node index to look for.

  int position A flag to pass through.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *FirstSegmentX(SegmentsX *segmentsx,index_t nodeindex,int position)
{
 index_t index=segmentsx->firstnode[nodeindex];
 SegmentX *segmentx;

 if(index==NO_SEGMENT)
    return(NULL);

 segmentx=LookupSegmentX(segmentsx,index,position);

 return(segmentx);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node index.

  SegmentX *NextSegmentX Returns a pointer to the next segment with the same id.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The current segment.

  index_t nodeindex The node index.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *NextSegmentX(SegmentsX *segmentsx,SegmentX *segmentx,index_t nodeindex)
{
#if SLIM
 int position=1+(segmentx-&segmentsx->cached[0]);
#endif

 if(segmentx->node1==nodeindex)
   {
    if(segmentsx->next1)
      {
       index_t index=IndexSegmentX(segmentsx,segmentx);

       if(segmentsx->next1[index]==NO_SEGMENT)
          return(NULL);

       segmentx=LookupSegmentX(segmentsx,segmentsx->next1[index],position);

       return(segmentx);
      }
    else
      {
#if SLIM
       index_t index=IndexSegmentX(segmentsx,segmentx);
       index++;

       if(index>=segmentsx->number)
          return(NULL);

       segmentx=LookupSegmentX(segmentsx,index,position);
#else
       segmentx++;

       if(IndexSegmentX(segmentsx,segmentx)>=segmentsx->number)
          return(NULL);
#endif

       if(segmentx->node1!=nodeindex)
          return(NULL);

       return(segmentx);
      }
   }
 else
   {
    if(segmentx->next2==NO_SEGMENT)
       return(NULL);

    return(LookupSegmentX(segmentsx,segmentx->next2,position));
   }
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Remove bad segments (duplicated, zero length or with missing nodes).

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx)
{
 index_t duplicate=0,loop=0,nonode=0,good=0,total=0;
 SegmentX segmentx;
 int fd;
 node_t prevnode1=NO_NODE_ID,prevnode2=NO_NODE_ID;
 distance_t prevdist=0;

 /* Print the start message */

 printf_first("Checking Segments: Segments=0 Duplicate=0 Loop=0 No-Node=0");

 /* Allocate the array of node flags */

 segmentsx->usednode=AllocBitMask(nodesx->number);

 assert(segmentsx->usednode); /* Check AllocBitMask() worked */

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t index1=IndexNodeX(nodesx,segmentx.node1);
    index_t index2=IndexNodeX(nodesx,segmentx.node2);

    if(prevnode1==segmentx.node1 && prevnode2==segmentx.node2)
      {
       if(!prevdist && !DISTANCE(segmentx.distance))
          logerror("Segment connecting nodes %"Pnode_t" and %"Pnode_t" is duplicated.\n",segmentx.node1,segmentx.node2);

       if(!prevdist && DISTANCE(segmentx.distance))
          logerror("Segment connecting nodes %"Pnode_t" and %"Pnode_t" is duplicated (discarded the area).\n",segmentx.node1,segmentx.node2);

       if(prevdist && !DISTANCE(segmentx.distance))
          logerror("Segment connecting nodes %"Pnode_t" and %"Pnode_t" is duplicated (discarded the non-area).\n",segmentx.node1,segmentx.node2);

       if(prevdist && DISTANCE(segmentx.distance))
          logerror("Segment connecting nodes %"Pnode_t" and %"Pnode_t" is duplicated (both are areas).\n",segmentx.node1,segmentx.node2);

       duplicate++;
      }
    else if(segmentx.node1==segmentx.node2)
      {
       logerror("Segment connects node %"Pnode_t" to itself.\n",segmentx.node1);

       loop++;
      }
    else if(index1==NO_NODE || index2==NO_NODE)
      {
       if(index1==NO_NODE && index2==NO_NODE)
          logerror("Segment connects nodes %"Pnode_t" and %"Pnode_t" but neither exist.\n",segmentx.node1,segmentx.node2);

       if(index1==NO_NODE && index2!=NO_NODE)
          logerror("Segment connects nodes %"Pnode_t" and %"Pnode_t" but the first one does not exist.\n",segmentx.node1,segmentx.node2);

       if(index1!=NO_NODE && index2==NO_NODE)
          logerror("Segment connects nodes %"Pnode_t" and %"Pnode_t" but the second one does not exist.\n",segmentx.node1,segmentx.node2);

       nonode++;
      }
    else
      {
       WriteFile(fd,&segmentx,sizeof(SegmentX));

       SetBit(segmentsx->usednode,index1);
       SetBit(segmentsx->usednode,index2);

       good++;

       prevnode1=segmentx.node1;
       prevnode2=segmentx.node2;
       prevdist=DISTANCE(segmentx.distance);
      }

    total++;

    if(!(total%10000))
       printf_middle("Checking Segments: Segments=%"Pindex_t" Duplicate=%"Pindex_t" Loop=%"Pindex_t" No-Node=%"Pindex_t,total,duplicate,loop,nonode);
   }

 segmentsx->number=good;

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Checked Segments: Segments=%"Pindex_t" Duplicate=%"Pindex_t" Loop=%"Pindex_t" No-Node=%"Pindex_t,total,duplicate,loop,nonode);
}


/*++++++++++++++++++++++++++++++++++++++
  Measure the segments and replace node/way ids with indexes.

  SegmentsX *segmentsx The set of segments to process.

  NodesX *nodesx The set of nodes to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void MeasureSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx)
{
 index_t index=0;
 int fd;
 SegmentX segmentx;

 /* Print the start message */

 printf_first("Measuring Segments: Segments=0");

 /* Map into memory /  open the file */

#if !SLIM
 nodesx->data=MapFile(nodesx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t node1=IndexNodeX(nodesx,segmentx.node1);
    index_t node2=IndexNodeX(nodesx,segmentx.node2);
    index_t way  =IndexWayX (waysx ,segmentx.way);

    NodeX *nodex1=LookupNodeX(nodesx,node1,1);
    NodeX *nodex2=LookupNodeX(nodesx,node2,2);

    /* Replace the node and way ids with their indexes */

    segmentx.node1=node1;
    segmentx.node2=node2;
    segmentx.way  =way;

    /* Set the distance but preserve the other flags */

    segmentx.distance|=DISTANCE(DistanceX(nodex1,nodex2));

    /* Write the modified segment */

    WriteFile(fd,&segmentx,sizeof(SegmentX));

    index++;

    if(!(index%10000))
       printf_middle("Measuring Segments: Segments=%"Pindex_t,index);
   }

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Free the other now-unneeded indexes */

 free(nodesx->idata);
 nodesx->idata=NULL;

 free(waysx->idata);
 waysx->idata=NULL;

 /* Unmap from memory / close the file */

#if !SLIM
 nodesx->data=UnmapFile(nodesx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
#endif

 /* Print the final message */

 printf_last("Measured Segments: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove the duplicate segments.

  SegmentsX *segmentsx The set of segments to modify.

  NodesX *nodesx The set of nodes to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void DeduplicateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx)
{
 index_t duplicate=0,good=0;
 index_t index=0;
 int fd,nprev=0;
 index_t prevnode1=NO_NODE,prevnode2=NO_NODE;
 SegmentX prevsegx[MAX_SEG_PER_NODE],segmentx;
 Way prevway[MAX_SEG_PER_NODE];

 /* Print the start message */

 printf_first("Deduplicating Segments: Segments=0 Duplicate=0");

 /* Map into memory / open the file */

#if !SLIM
 waysx->data=MapFile(waysx->filename);
#else
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    WayX *wayx=LookupWayX(waysx,segmentx.way,1);
    int isduplicate=0;

    if(segmentx.node1==prevnode1 && segmentx.node2==prevnode2)
      {
       int offset;

       for(offset=0;offset<nprev;offset++)
         {
          if(DISTFLAG(segmentx.distance)==DISTFLAG(prevsegx[offset].distance))
             if(!WaysCompare(&prevway[offset],&wayx->way))
               {
                isduplicate=1;
                break;
               }
         }

       if(isduplicate)
         {
          nprev--;

          for(;offset<nprev;offset++)
            {
             prevsegx[offset]=prevsegx[offset+1];
             prevway[offset] =prevway[offset+1];
            }
         }
       else
         {
          assert(nprev<MAX_SEG_PER_NODE); /* Only a limited amount of information stored. */

          prevsegx[nprev]=segmentx;
          prevway[nprev] =wayx->way;

          nprev++;
         }
      }
    else
      {
       nprev=1;
       prevnode1=segmentx.node1;
       prevnode2=segmentx.node2;
       prevsegx[0]=segmentx;
       prevway[0] =wayx->way;
      }

    if(isduplicate)
       duplicate++;
    else
      {
       WriteFile(fd,&segmentx,sizeof(SegmentX));

       good++;
      }

    index++;

    if(!(index%10000))
       printf_middle("Deduplicating Segments: Segments=%"Pindex_t" Duplicate=%"Pindex_t,index,duplicate);
   }

 segmentsx->number=good;

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Unmap from memory / close the file */

#if !SLIM
 waysx->data=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Deduplicated Segments: Segments=%"Pindex_t" Duplicate=%"Pindex_t" Unique=%"Pindex_t,index,duplicate,good);
}


/*++++++++++++++++++++++++++++++++++++++
  Index the segments by creating the firstnode index and filling in the segment next2 parameter.

  SegmentsX *segmentsx The set of segments to modify.

  NodesX *nodesx The sset of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexSegments(SegmentsX *segmentsx,NodesX *nodesx)
{
 index_t index,i;

 if(segmentsx->number==0)
    return;

 /* Print the start message */

 printf_first("Indexing Segments: Segments=0");

 /* Allocate the array of indexes */

 if(!segmentsx->firstnode)
   {
    segmentsx->firstnode=(index_t*)malloc(nodesx->number*sizeof(index_t));

    assert(segmentsx->firstnode); /* Check malloc() worked */
   }

 for(i=0;i<nodesx->number;i++)
    segmentsx->firstnode[i]=NO_SEGMENT;

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->data=MapFileWriteable(segmentsx->filename);
#else
 segmentsx->fd=ReOpenFileWriteable(segmentsx->filename);
#endif

 /* Read through the segments in reverse order */

 for(index=segmentsx->number-1;index!=NO_SEGMENT;index--)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,index,1);

    segmentx->next2=segmentsx->firstnode[segmentx->node2];

    PutBackSegmentX(segmentsx,segmentx);

    segmentsx->firstnode[segmentx->node1]=index;
    segmentsx->firstnode[segmentx->node2]=index;

    if(!(index%10000))
       printf_middle("Indexing Segments: Segments=%"Pindex_t,segmentsx->number-index);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->data=UnmapFile(segmentsx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Indexed Segments: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Update the segment indexes after geographical sorting.

  SegmentsX *segmentsx The set of segments to modify.

  NodesX *nodesx The set of nodes to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx)
{
 index_t i;
 int fd;

 /* Print the start message */

 printf_first("Updating Segments: Segments=0");

 /* Map into memory / open the files */

#if !SLIM
 waysx->data=MapFile(waysx->filename);
#else
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX segmentx;
    WayX *wayx;

    ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

    segmentx.node1=nodesx->gdata[segmentx.node1];
    segmentx.node2=nodesx->gdata[segmentx.node2];

    if(segmentx.node1>segmentx.node2)
      {
       index_t temp;

       temp=segmentx.node1;
       segmentx.node1=segmentx.node2;
       segmentx.node2=temp;

       if(segmentx.distance&(ONEWAY_2TO1|ONEWAY_1TO2))
          segmentx.distance^=ONEWAY_2TO1|ONEWAY_1TO2;
      }

    wayx=LookupWayX(waysx,segmentx.way,1);

    segmentx.way=wayx->prop;

    WriteFile(fd,&segmentx,sizeof(SegmentX));

    if(!((i+1)%10000))
       printf_middle("Updating Segments: Segments=%"Pindex_t,i+1);
   }

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Unmap from memory / close the files */

#if !SLIM
 waysx->data=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Updated Segments: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the segment list to a file.

  SegmentsX *segmentsx The set of segments to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveSegmentList(SegmentsX *segmentsx,const char *filename)
{
 index_t i;
 int fd;
 SegmentsFile segmentsfile={0};
 index_t super_number=0,normal_number=0;

 /* Print the start message */

 printf_first("Writing Segments: Segments=0");

 /* Re-open the file */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 /* Write out the segments data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(SegmentsFile));

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX segmentx;
    Segment  segment={0};

    ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

    segment.node1   =segmentx.node1;
    segment.node2   =segmentx.node2;
    segment.next2   =segmentx.next2;
    segment.way     =segmentx.way;
    segment.distance=segmentx.distance;

    if(IsSuperSegment(&segment))
       super_number++;
    if(IsNormalSegment(&segment))
       normal_number++;

    WriteFile(fd,&segment,sizeof(Segment));

    if(!((i+1)%10000))
       printf_middle("Writing Segments: Segments=%"Pindex_t,i+1);
   }

 /* Write out the header structure */

 segmentsfile.number=segmentsx->number;
 segmentsfile.snumber=super_number;
 segmentsfile.nnumber=normal_number;

 SeekFile(fd,0);
 WriteFile(fd,&segmentsfile,sizeof(SegmentsFile));

 CloseFile(fd);

 /* Close the file */

 segmentsx->fd=CloseFile(segmentsx->fd);

 /* Print the final message */

 printf_last("Wrote Segments: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two nodes.

  distance_t DistanceX Returns the distance between the extended nodes.

  NodeX *nodex1 The starting node.

  NodeX *nodex2 The end node.
  ++++++++++++++++++++++++++++++++++++++*/

static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2)
{
 double dlon = latlong_to_radians(nodex1->longitude) - latlong_to_radians(nodex2->longitude);
 double dlat = latlong_to_radians(nodex1->latitude)  - latlong_to_radians(nodex2->latitude);
 double lat1 = latlong_to_radians(nodex1->latitude);
 double lat2 = latlong_to_radians(nodex2->latitude);

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (lat1) * cos (lat2) * a2 * a2;
 sa = sqrt (a);
 if (sa <= 1.0)
   {c = 2 * asin (sa);}
 else
   {c = 2 * asin (1.0);}
 d = 6378.137 * c;

 return km_to_distance(d);
}
