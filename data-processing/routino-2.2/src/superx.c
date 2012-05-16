/***************************************
 Super-Segment data type functions.

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

#include "types.h"
#include "segments.h"
#include "ways.h"

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"

#include "files.h"
#include "logging.h"
#include "results.h"


/* Local functions */

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match);


/*++++++++++++++++++++++++++++++++++++++
  Select the super-nodes from the list of nodes.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 index_t i;
 index_t nnodes=0;

 if(nodesx->number==0 || segmentsx->number==0 || waysx->number==0)
    return;

 /* Print the start message */

 printf_first("Finding Super-Nodes: Nodes=0 Super-Nodes=0");

 /* Allocate and set the super-node markers */

 if(!nodesx->super)
   {
    nodesx->super=AllocBitMask(nodesx->number);

    assert(nodesx->super); /* Check AllocBitMask() worked */

    SetAllBits1(nodesx->super,nodesx->number);
   }

 /* Map into memory / open the files */

#if !SLIM
 nodesx->data=MapFile(nodesx->filename);
 segmentsx->data=MapFile(segmentsx->filename);
 waysx->data=MapFile(waysx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Find super-nodes */

 for(i=0;i<nodesx->number;i++)
   {
    if(IsBitSet(nodesx->super,i))
      {
       int issuper=0;
       NodeX *nodex=LookupNodeX(nodesx,i,1);

       if(IsPrunedNodeX(nodex))
          issuper=0;
       else if(nodex->flags&(NODE_TURNRSTRCT|NODE_TURNRSTRCT2))
          issuper=1;
       else
         {
          int count=0,j;
          Way segmentway[MAX_SEG_PER_NODE];
          int segmentweight[MAX_SEG_PER_NODE];
          SegmentX *segmentx=FirstSegmentX(segmentsx,i,1);

          while(segmentx)
            {
             WayX *wayx=LookupWayX(waysx,segmentx->way,1);
             int nsegments;

             /* Segments that are loops count twice */

             assert(count<MAX_SEG_PER_NODE); /* Only a limited amount of information stored. */

             if(segmentx->node1==segmentx->node2)
                segmentweight[count]=2;
             else
                segmentweight[count]=1;

             segmentway[count]=wayx->way;

             /* If the node allows less traffic types than any connecting way then it is super */

             if((wayx->way.allow&nodex->allow)!=wayx->way.allow)
               {
                issuper=1;
                break;
               }

             nsegments=segmentweight[count];

             for(j=0;j<count;j++)
                if(wayx->way.allow & segmentway[j].allow)
                  {
                   /* If two ways are different in any attribute and there is a type of traffic that can use both then it is super */

                   if(WaysCompare(&segmentway[j],&wayx->way))
                     {
                      issuper=1;
                      break;
                     }

                   /* If there are two other segments that can be used by the same types of traffic as this one then it is super */

                   nsegments+=segmentweight[j];
                   if(nsegments>2)
                     {
                      issuper=1;
                      break;
                     }
                  }

             if(issuper)
                break;

             segmentx=NextSegmentX(segmentsx,segmentx,i);

             count++;
            }
         }

       /* Mark the node as super if it is. */

       if(issuper)
          nnodes++;
       else
          ClearBit(nodesx->super,i);
      }

    if(!((i+1)%10000))
       printf_middle("Finding Super-Nodes: Nodes=%"Pindex_t" Super-Nodes=%"Pindex_t,i+1,nnodes);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 nodesx->data=UnmapFile(nodesx->filename);
 segmentsx->data=UnmapFile(segmentsx->filename);
 waysx->data=UnmapFile(waysx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 segmentsx->fd=CloseFile(segmentsx->fd);
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Found Super-Nodes: Nodes=%"Pindex_t" Super-Nodes=%"Pindex_t,nodesx->number,nnodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the super-segments from the existing segments.

  SegmentsX *CreateSuperSegments Returns the new super segments.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 index_t i;
 SegmentsX *supersegmentsx;
 index_t sn=0,ss=0;

 supersegmentsx=NewSegmentList(0);

 if(segmentsx->number==0 || waysx->number==0)
    return(supersegmentsx);

 /* Print the start message */

 printf_first("Creating Super-Segments: Super-Nodes=0 Super-Segments=0");

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->data=MapFile(segmentsx->filename);
 waysx->data=MapFile(waysx->filename);
#else
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Create super-segments for each super-node. */

 for(i=0;i<nodesx->number;i++)
   {
    if(IsBitSet(nodesx->super,i))
      {
       SegmentX *segmentx;
       int count=0,match;
       Way prevway[MAX_SEG_PER_NODE];

       segmentx=FirstSegmentX(segmentsx,i,1);

       while(segmentx)
         {
          WayX *wayx=LookupWayX(waysx,segmentx->way,1);

          /* Check that this type of way hasn't already been routed */

          match=0;

          if(count>0)
            {
             int j;

             for(j=0;j<count;j++)
                if(!WaysCompare(&prevway[j],&wayx->way))
                  {
                   match=1;
                   break;
                  }
            }

          assert(count<MAX_SEG_PER_NODE); /* Only a limited amount of history stored. */

          prevway[count++]=wayx->way;

          /* Route the way and store the super-segments. */

          if(!match)
            {
             Results *results=FindRoutesWay(nodesx,segmentsx,waysx,i,&wayx->way);
             Result *result=FirstResult(results);

             while(result)
               {
                if(IsBitSet(nodesx->super,result->node) && result->segment!=NO_SEGMENT)
                  {
                   if(wayx->way.type&Way_OneWay && result->node!=i)
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score)|ONEWAY_1TO2);
                   else
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score));

                   ss++;
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentx=NextSegmentX(segmentsx,segmentx,i);
         }

       sn++;

       if(!(sn%10000))
          printf_middle("Creating Super-Segments: Super-Nodes=%"Pindex_t" Super-Segments=%"Pindex_t,sn,ss);
      }
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->data=UnmapFile(segmentsx->filename);
 waysx->data=UnmapFile(waysx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Created Super-Segments: Super-Nodes=%"Pindex_t" Super-Segments=%"Pindex_t,sn,ss);

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Merge the segments and super-segments into a new segment list.

  SegmentsX *MergeSuperSegments Returns a new set of merged segments.

  SegmentsX *segmentsx The set of segments to use.

  SegmentsX *supersegmentsx The set of super-segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *MergeSuperSegments(SegmentsX *segmentsx,SegmentsX *supersegmentsx)
{
 index_t i,j;
 index_t merged=0,added=0;
 SegmentsX *mergedsegmentsx;

 mergedsegmentsx=NewSegmentList(0);

 if(segmentsx->number==0)
    return(mergedsegmentsx);

 /* Print the start message */

 printf_first("Merging Segments: Segments=0 Super=0 Merged=0 Added=0");

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->data=MapFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->data=MapFile(supersegmentsx->filename);
#else
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->fd=ReOpenFile(supersegmentsx->filename);
#endif

 /* Loop through and create a new list of combined segments */

 for(i=0,j=0;i<segmentsx->number;i++)
   {
    int super=0;
    SegmentX *segmentx=LookupSegmentX(segmentsx,i,1);

    while(j<supersegmentsx->number)
      {
       SegmentX *supersegmentx=LookupSegmentX(supersegmentsx,j,1);

       if(segmentx->node1   ==supersegmentx->node1 &&
          segmentx->node2   ==supersegmentx->node2 &&
          segmentx->distance==supersegmentx->distance)
         {
          merged++;
          j++;
          /* mark as super-segment and normal segment */
          super=1;
          break;
         }
       else if((segmentx->node1==supersegmentx->node1 &&
                segmentx->node2==supersegmentx->node2) ||
               (segmentx->node1==supersegmentx->node1 &&
                segmentx->node2>supersegmentx->node2) ||
               (segmentx->node1>supersegmentx->node1))
         {
          /* mark as super-segment */
          AppendSegment(mergedsegmentsx,supersegmentx->way,supersegmentx->node1,supersegmentx->node2,supersegmentx->distance|SEGMENT_SUPER);
          added++;
          j++;
         }
       else
         {
          /* mark as normal segment */
          break;
         }
      }

    if(super)
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_SUPER|SEGMENT_NORMAL);
    else
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_NORMAL);

    if(!((i+1)%10000))
       printf_middle("Merging Segments: Segments=%"Pindex_t" Super=%"Pindex_t" Merged=%"Pindex_t" Added=%"Pindex_t,i+1,j,merged,added);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->data=UnmapFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->data=UnmapFile(supersegmentsx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
 if(supersegmentsx->number>0)
    supersegmentsx->fd=CloseFile(supersegmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Merged Segments: Segments=%"Pindex_t" Super=%"Pindex_t" Merged=%"Pindex_t" Added=%"Pindex_t,segmentsx->number,supersegmentsx->number,merged,added);

 return(mergedsegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified super-node to any other super-node that follows a certain type of way.

  Results *FindRoutesWay Returns a set of results.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.

  node_t start The start node.

  Way *match A template for the type of way that the route must follow.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match)
{
 Results *results;
 Queue *queue;
 Result *result1,*result2;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(4);

 queue=NewQueueList();

 result1=InsertResult(results,start,NO_SEGMENT);

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1;
    SegmentX *segmentx;

    node1=result1->node;

    segmentx=FirstSegmentX(segmentsx,node1,2); /* position 1 is already used */

    while(segmentx)
      {
       index_t node2,seg2;
       distance_t cumulative_distance;

       /* must not be one-way against the direction of travel */
       if(IsOnewayTo(segmentx,node1))
          goto endloop;

       node2=OtherNode(segmentx,node1);

       seg2=IndexSegmentX(segmentsx,segmentx);

       /* must not be a u-turn */
       if(result1->segment==seg2)
          goto endloop;

       wayx=LookupWayX(waysx,segmentx->way,2); /* position 1 is already used */

       /* must be the right type of way */
       if(WaysCompare(&wayx->way,match))
          goto endloop;

       cumulative_distance=(distance_t)result1->score+DISTANCE(segmentx->distance);

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          /* don't route beyond a super-node. */
          if(!IsBitSet(nodesx->super,node2))
             InsertInQueue(queue,result2);
         }
       else if(cumulative_distance<result2->score)
         {
          result2->prev=result1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          /* don't route beyond a super-node. */
          if(!IsBitSet(nodesx->super,node2))
             InsertInQueue(queue,result2);
         }

      endloop:

       segmentx=NextSegmentX(segmentsx,segmentx,node1);
      }
   }

 FreeQueueList(queue);

 return(results);
}
