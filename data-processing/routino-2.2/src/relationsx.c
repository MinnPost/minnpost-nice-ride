/***************************************
 Extended Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2012 Andrew M. Bishop

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
#include "segments.h"
#include "relations.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Local functions */

static int sort_by_id(TurnRestrictRelX *a,TurnRestrictRelX *b);
static int deduplicate_by_id(TurnRestrictRelX *relationx,index_t index);

static int sort_by_via(TurnRestrictRelX *a,TurnRestrictRelX *b);


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new relation list (create a new file or open an existing one).

  RelationsX *NewRelationList Returns the relation list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

RelationsX *NewRelationList(int append)
{
 RelationsX *relationsx;

 relationsx=(RelationsX*)calloc(1,sizeof(RelationsX));

 assert(relationsx); /* Check calloc() worked */


 /* Route Relations */

 relationsx->rfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->rfilename,"%s/relationsx.route.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->rfilename,"%s/relationsx.route.%p.tmp",option_tmpdirname,(void*)relationsx);

 if(append)
   {
    off_t size,position=0;

    relationsx->rfd=OpenFileAppend(relationsx->rfilename);

    size=SizeFile(relationsx->rfilename);

    while(position<size)
      {
       FILESORT_VARINT relationsize;

       SeekReadFile(relationsx->rfd,&relationsize,FILESORT_VARSIZE,position);

       relationsx->rnumber++;
       position+=relationsize+FILESORT_VARSIZE;
      }

    SeekFile(relationsx->rfd,size);
   }
 else
    relationsx->rfd=OpenFileNew(relationsx->rfilename);


 /* Turn Restriction Relations */

 relationsx->trfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->trfilename,"%s/relationsx.turn.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->trfilename,"%s/relationsx.turn.%p.tmp",option_tmpdirname,(void*)relationsx);

 if(append)
   {
    off_t size;

    relationsx->trfd=OpenFileAppend(relationsx->trfilename);

    size=SizeFile(relationsx->trfilename);

    relationsx->trnumber=size/sizeof(TurnRestrictRelX);
   }
 else
    relationsx->trfd=OpenFileNew(relationsx->trfilename);

 return(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a relation list.

  RelationsX *relationsx The set of relations to be freed.

  int keep Set to 1 if the file is to be kept (for appending later).
  ++++++++++++++++++++++++++++++++++++++*/

void FreeRelationList(RelationsX *relationsx,int keep)
{
 /* Route relations */

 if(!keep)
    DeleteFile(relationsx->rfilename);

 free(relationsx->rfilename);


 /* Turn Restriction relations */

 if(!keep)
    DeleteFile(relationsx->trfilename);

 free(relationsx->trfilename);

 free(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted route relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  transports_t routes The types of routes that this relation is for.

  way_t *ways The array of ways that are members of the relation.

  int nways The number of ways that are members of the relation.

  relation_t *relations The array of relations that are members of the relation.

  int nrelations The number of relations that are members of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendRouteRelation(RelationsX* relationsx,relation_t id,
                         transports_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations)
{
 RouteRelX relationx;
 FILESORT_VARINT size;
 way_t noway=NO_WAY;
 relation_t norelation=NO_RELATION;

 relationx.id=id;
 relationx.routes=routes;

 size=sizeof(RouteRelX)+(nways+1)*sizeof(way_t)+(nrelations+1)*sizeof(relation_t);

 WriteFile(relationsx->rfd,&size,FILESORT_VARSIZE);
 WriteFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

 WriteFile(relationsx->rfd,ways  ,nways*sizeof(way_t));
 WriteFile(relationsx->rfd,&noway,      sizeof(way_t));

 WriteFile(relationsx->rfd,relations  ,nrelations*sizeof(relation_t));
 WriteFile(relationsx->rfd,&norelation,           sizeof(relation_t));

 relationsx->rnumber++;

 assert(!(relationsx->rnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted turn restriction relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  way_t from The way that the turn restriction starts from.

  way_t to The way that the restriction finished on.

  node_t via The node that the turn restriction passes through.

  TurnRestriction restriction The type of restriction.

  transports_t except The set of transports allowed to bypass the restriction.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendTurnRestrictRelation(RelationsX* relationsx,relation_t id,
                                way_t from,way_t to,node_t via,
                                TurnRestriction restriction,transports_t except)
{
 TurnRestrictRelX relationx={0};

 relationx.id=id;
 relationx.from=from;
 relationx.to=to;
 relationx.via=via;
 relationx.restriction=restriction;
 relationx.except=except;

 WriteFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

 relationsx->trnumber++;

 assert(!(relationsx->trnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortRelationList(RelationsX* relationsx)
{
 /* Close the files (finished appending) */

 relationsx->rfd=CloseFile(relationsx->rfd);

 relationsx->trfd=CloseFile(relationsx->trfd);


 /* Route Relations */


 /* Turn Restriction Relations. */

 if(relationsx->trnumber)
   {
    index_t trxnumber;
    int trfd;

    /* Print the start message */

    printf_first("Sorting Turn Relations");

    /* Re-open the file read-only and a new file writeable */

    relationsx->trfd=ReOpenFile(relationsx->trfilename);

    DeleteFile(relationsx->trfilename);

    trfd=OpenFileNew(relationsx->trfilename);

    /* Sort the relations */

    trxnumber=relationsx->trnumber;
    relationsx->trnumber=0;

    relationsx->trnumber=filesort_fixed(relationsx->trfd,trfd,sizeof(TurnRestrictRelX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))deduplicate_by_id);

    /* Close the files */

    relationsx->trfd=CloseFile(relationsx->trfd);
    CloseFile(trfd);

    /* Print the final message */

    printf_last("Sorted Turn Relations: Relations=%"Pindex_t" Duplicates=%"Pindex_t,trxnumber,trxnumber-relationsx->trnumber);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the turn restriction relations into id order.

  int sort_by_id Returns the comparison of the id fields.

  TurnRestrictRelX *a The first extended relation.

  TurnRestrictRelX *b The second extended relation.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(TurnRestrictRelX *a,TurnRestrictRelX *b)
{
 relation_t a_id=a->id;
 relation_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Deduplicate the extended relations using the id after sorting.

  int deduplicate_by_id Return 1 if the value is to be kept, otherwise 0.

  TurnRestrictRelX *relationx The extended relation.

  index_t index The index of this relation in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int deduplicate_by_id(TurnRestrictRelX *relationx,index_t index)
{
 static relation_t previd;

 if(index==0 || relationx->id!=previd)
   {
    previd=relationx->id;

    return(1);
   }
 else
   {
    logerror("Relation %"Prelation_t" is duplicated.\n",relationx->id);

    return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of turn relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortTurnRelationList(RelationsX* relationsx)
{
 int trfd;

 if(relationsx->trnumber==0)
    return;

 /* Print the start message */

 printf_first("Sorting Turn Relations");

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Sort the relations */

 filesort_fixed(relationsx->trfd,trfd,sizeof(TurnRestrictRelX),(int (*)(const void*,const void*))sort_by_via,NULL);

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Print the final message */

 printf_last("Sorted Turn Relations: Relations=%"Pindex_t,relationsx->trnumber);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the turn restriction relations into via index order (then by from and to segments).

  int sort_by_via Returns the comparison of the via, from and to fields.

  TurnRestrictRelX *a The first extended relation.

  TurnRestrictRelX *b The second extended relation.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_via(TurnRestrictRelX *a,TurnRestrictRelX *b)
{
 index_t a_id=a->via;
 index_t b_id=b->via;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
   {
    index_t a_id=a->from;
    index_t b_id=b->from;

    if(a_id<b_id)
       return(-1);
    else if(a_id>b_id)
       return(1);
    else
      {
       index_t a_id=a->to;
       index_t b_id=b->to;

       if(a_id<b_id)
          return(-1);
       else if(a_id>b_id)
          return(1);
       else
          return(0);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Process the route relations and apply the information to the ways.

  RelationsX *relationsx The set of relations to use.

  WaysX *waysx The set of ways to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx)
{
 RouteRelX *unmatched=NULL,*lastunmatched=NULL;
 int nunmatched=0,lastnunmatched=0,iteration=1;

 if(waysx->number==0)
    return;

 /* Map into memory / open the files */

#if !SLIM
 waysx->data=MapFileWriteable(waysx->filename);
#else
 waysx->fd=ReOpenFileWriteable(waysx->filename);
#endif

 /* Re-open the file read-only */

 relationsx->rfd=ReOpenFile(relationsx->rfilename);

 /* Read through the file. */

 do
   {
    int ways=0,relations=0;
    index_t i;

    SeekFile(relationsx->rfd,0);

    /* Print the start message */

    printf_first("Processing Route Relations (%d): Relations=0 Modified Ways=0",iteration);

    for(i=0;i<relationsx->rnumber;i++)
      {
       FILESORT_VARINT size;
       RouteRelX relationx;
       way_t wayid;
       relation_t relationid;
       transports_t routes=Transports_None;

       /* Read each route relation */

       ReadFile(relationsx->rfd,&size,FILESORT_VARSIZE);
       ReadFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

       /* Decide what type of route it is */

       if(iteration==1)
         {
          relations++;
          routes=relationx.routes;
         }
       else
         {
          int j;

          for(j=0;j<lastnunmatched;j++)
             if(lastunmatched[j].id==relationx.id)
               {
                relations++;

                if((lastunmatched[j].routes|relationx.routes)==relationx.routes)
                   routes=0; /* Nothing new to add */
                else
                   routes=lastunmatched[j].routes;

                break;
               }
         }

       /* Loop through the ways */

       do
         {
          ReadFile(relationsx->rfd,&wayid,sizeof(way_t));

          /* Update the ways that are listed for the relation */

          if(wayid==NO_WAY)
             continue;

          if(routes)
            {
             index_t way=IndexWayX(waysx,wayid);

             if(way!=NO_WAY)
               {
                WayX *wayx=LookupWayX(waysx,way,1);

                if(routes&Transports_Foot)
                   wayx->way.props|=Properties_FootRoute;

                if(routes&Transports_Bicycle)
                   wayx->way.props|=Properties_BicycleRoute;

                PutBackWayX(waysx,wayx);

                ways++;
               }
             else
                logerror("Route Relation %"Prelation_t" contains Way %"Pway_t" but it does not exist in the Routino database.\n",relationx.id,wayid);
            }
         }
       while(wayid!=NO_WAY);

       /* Loop through the relations */

       do
         {
          ReadFile(relationsx->rfd,&relationid,sizeof(relation_t));

          /* Add the relations that are listed for this relation to the list for next time */

          if(relationid==NO_RELATION)
             continue;

          if(relationid==relationx.id)
             logerror("Relation %"Prelation_t" contains itself.\n",relationx.id);
          else if(routes)
            {
             if(nunmatched%256==0)
                unmatched=(RouteRelX*)realloc((void*)unmatched,(nunmatched+256)*sizeof(RouteRelX));

             unmatched[nunmatched].id=relationid;
             unmatched[nunmatched].routes=routes;

             nunmatched++;
            }
         }
       while(relationid!=NO_RELATION);

       if(!((i+1)%1000))
          printf_middle("Processing Route Relations (%d): Relations=%"Pindex_t" Modified Ways=%"Pindex_t,iteration,relations,ways);
      }

    if(lastunmatched)
       free(lastunmatched);

    lastunmatched=unmatched;
    lastnunmatched=nunmatched;

    unmatched=NULL;
    nunmatched=0;

    /* Print the final message */

    printf_last("Processed Route Relations (%d): Relations=%"Pindex_t" Modified Ways=%"Pindex_t,iteration,relations,ways);
   }
 while(lastnunmatched && iteration++<8);

 if(lastunmatched)
    free(lastunmatched);

 /* Close the file */

 relationsx->rfd=CloseFile(relationsx->rfd);

 /* Unmap from memory / close the files */

#if !SLIM
 waysx->data=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif
}


/*++++++++++++++++++++++++++++++++++++++
  Process the turn relations (first part) to update them with the node/way information.

  RelationsX *relationsx The set of relations to modify.

  NodesX *nodesx The set of nodes to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessTurnRelations1(RelationsX *relationsx,NodesX *nodesx,WaysX *waysx)
{
 int trfd;
 index_t i,deleted=0;

 /* Print the start message */

 printf_first("Processing Turn Relations (1): Relations=0");

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Process all of the relations */

 for(i=0;i<relationsx->trnumber;i++)
   {
    TurnRestrictRelX relationx;
    node_t via;
    way_t from,to;

    ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

    via =IndexNodeX(nodesx,relationx.via);
    from=IndexWayX(waysx,relationx.from);
    to  =IndexWayX(waysx,relationx.to);

    if(via==NO_NODE)
       logerror("Turn Relation %"Prelation_t" contains Node %"Pnode_t" but it does not exist in the Routino database.\n",relationx.id,relationx.via);

    if(from==NO_WAY)
       logerror("Turn Relation %"Prelation_t" contains Way %"Pway_t" but it does not exist in the Routino database.\n",relationx.id,relationx.from);

    if(to==NO_WAY)
       logerror("Turn Relation %"Prelation_t" contains Way %"Pway_t" but it does not exist in the Routino database.\n",relationx.id,relationx.to);

    relationx.via =via;
    relationx.from=from;
    relationx.to  =to;

    if(relationx.via==NO_NODE || relationx.from==NO_WAY || relationx.to==NO_WAY)
       deleted++;
    else
       WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

    if(!((i+1)%1000))
       printf_middle("Processing Turn Relations (1): Relations=%"Pindex_t" Deleted=%"Pindex_t,i+1-deleted,deleted);
   }

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Print the final message */

 printf_last("Processed Turn Relations (1): Relations=%"Pindex_t" Deleted=%"Pindex_t,relationsx->trnumber-deleted,deleted);

 relationsx->trnumber-=deleted;
}


/*++++++++++++++++++++++++++++++++++++++
  Process the turn relations (second part) to convert them to nodes.

  RelationsX *relationsx The set of relations to modify.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessTurnRelations2(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 TurnRestrictRelX relationx;
 int trfd;
 index_t total=0,deleted=0;

 if(nodesx->number==0 || segmentsx->number==0)
    return;

 /* Print the start message */

 printf_first("Processing Turn Relations (2): Relations=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->data=MapFileWriteable(nodesx->filename);
 segmentsx->data=MapFile(segmentsx->filename);
 waysx->data=MapFile(waysx->filename);
#else
 nodesx->fd=ReOpenFileWriteable(nodesx->filename);
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Process all of the relations */

 while(!ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX)))
   {
    NodeX *nodex;
    SegmentX *segmentx;

    if(relationx.restriction==TurnRestrict_no_right_turn ||
       relationx.restriction==TurnRestrict_no_left_turn ||
       relationx.restriction==TurnRestrict_no_u_turn ||
       relationx.restriction==TurnRestrict_no_straight_on)
      {
       index_t node_from=NO_NODE,node_to=NO_NODE;
       int oneway_from=0,oneway_to=0,vehicles_from=1,vehicles_to=1;

       /* Find the segments that join the node 'via' */

       segmentx=FirstSegmentX(segmentsx,relationx.via,1);

       while(segmentx)
         {
          if(segmentx->way==relationx.from)
            {
             WayX *wayx=LookupWayX(waysx,segmentx->way,1);

             if(node_from!=NO_NODE) /* Only one segment can be on the 'from' way */
               {
                logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not at the end of the 'from' way.\n",relationx.id);
                deleted++;
                goto endloop;
               }

             node_from=OtherNode(segmentx,relationx.via);

             if(IsOnewayFrom(segmentx,relationx.via))
                oneway_from=1;  /* not allowed */

             if(!(wayx->way.allow&(Transports_Bicycle|Transports_Moped|Transports_Motorbike|Transports_Motorcar|Transports_Goods|Transports_HGV|Transports_PSV)))
                vehicles_from=0;  /* not allowed */
            }

          if(segmentx->way==relationx.to)
            {
             WayX *wayx=LookupWayX(waysx,segmentx->way,1);

             if(node_to!=NO_NODE) /* Only one segment can be on the 'to' way */
               {
                logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not at the end of the 'to' way.\n",relationx.id);
                deleted++;
                goto endloop;
               }

             node_to=OtherNode(segmentx,relationx.via);

             if(IsOnewayTo(segmentx,relationx.via))
                oneway_to=1;  /* not allowed */

             if(!(wayx->way.allow&(Transports_Bicycle|Transports_Moped|Transports_Motorbike|Transports_Motorcar|Transports_Goods|Transports_HGV|Transports_PSV)))
                vehicles_to=0;  /* not allowed */
            }

          segmentx=NextSegmentX(segmentsx,segmentx,relationx.via);
         }

       if(node_from==NO_NODE)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not part of the 'from' way.\n",relationx.id);

       if(node_to==NO_NODE)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not part of the 'to' way.\n",relationx.id);

       if(oneway_from)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'from' way is oneway away from the 'via' node.\n",relationx.id);

       if(oneway_to)
          logerror("Turn Relation %"Prelation_t" is not needed because the 'to' way is oneway towards the 'via' node.\n",relationx.id);

       if(!vehicles_from)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'from' way does not allow vehicles.\n",relationx.id);

       if(!vehicles_to)
          logerror("Turn Relation %"Prelation_t" is not needed because the 'to' way does not allow vehicles.\n",relationx.id);

       if(oneway_from || oneway_to || !vehicles_from || !vehicles_to || node_from==NO_NODE || node_to==NO_NODE)
         {
          deleted++;
          goto endloop;
         }

       /* Write the results */

       relationx.from=node_from;
       relationx.to  =node_to;

       WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

       total++;

       if(!(total%1000))
          printf_middle("Processing Turn Relations (2): Relations=%"Pindex_t" Deleted=%"Pindex_t" Added=%"Pindex_t,total,deleted,total-relationsx->trnumber+deleted);
      }
    else
      {
       index_t node_from=NO_NODE,node_to=NO_NODE,node_other[MAX_SEG_PER_NODE];
       int nnodes_other=0,i;
       int oneway_from=0,vehicles_from=1;

       /* Find the segments that join the node 'via' */

       segmentx=FirstSegmentX(segmentsx,relationx.via,1);

       while(segmentx)
         {
          if(segmentx->way==relationx.from)
            {
             WayX *wayx=LookupWayX(waysx,segmentx->way,1);

             if(node_from!=NO_NODE) /* Only one segment can be on the 'from' way */
               {
                logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not at the end of the 'from' way.\n",relationx.id);
                deleted++;
                goto endloop;
               }

             node_from=OtherNode(segmentx,relationx.via);

             if(IsOnewayFrom(segmentx,relationx.via))
                oneway_from=1;  /* not allowed */

             if(!(wayx->way.allow&(Transports_Bicycle|Transports_Moped|Transports_Motorbike|Transports_Motorcar|Transports_Goods|Transports_HGV|Transports_PSV)))
                vehicles_from=0;  /* not allowed */
            }

          if(segmentx->way==relationx.to)
            {
             if(node_to!=NO_NODE) /* Only one segment can be on the 'to' way */
               {
                logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not at the end of the 'to' way.\n",relationx.id);
                deleted++;
                goto endloop;
               }

             node_to=OtherNode(segmentx,relationx.via);
            }

          if(segmentx->way!=relationx.from && segmentx->way!=relationx.to)
            {
             WayX *wayx=LookupWayX(waysx,segmentx->way,1);

             if(IsOnewayTo(segmentx,relationx.via))
                ;  /* not allowed */
             else if(!(wayx->way.allow&(Transports_Bicycle|Transports_Moped|Transports_Motorbike|Transports_Motorcar|Transports_Goods|Transports_HGV|Transports_PSV)))
                ;  /* not allowed */
             else
               {
                assert(nnodes_other<MAX_SEG_PER_NODE); /* Only a limited amount of information stored. */

                node_other[nnodes_other++]=OtherNode(segmentx,relationx.via);
               }
            }

          segmentx=NextSegmentX(segmentsx,segmentx,relationx.via);
         }

       if(node_from==NO_NODE)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not part of the 'from' way.\n",relationx.id);

       if(node_to==NO_NODE)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'via' node is not part of the 'to' way.\n",relationx.id);

       if(nnodes_other==0)
          logerror("Turn Relation %"Prelation_t" is not needed because the only allowed exit from the 'via' node is the 'to' way.\n",relationx.id);

       if(oneway_from)
          logerror("Turn Relation %"Prelation_t" is not needed because the 'from' way is oneway away from the 'via' node.\n",relationx.id);

       if(!vehicles_from)
          logerror("Turn Relation %"Prelation_t" is not stored because the 'from' way does not allow vehicles.\n",relationx.id);

       if(oneway_from || !vehicles_from || node_from==NO_NODE || node_to==NO_NODE || nnodes_other==0)
         {
          deleted++;
          goto endloop;
         }

       /* Write the results */

       for(i=0;i<nnodes_other;i++)
         {
          relationx.from=node_from;
          relationx.to  =node_other[i];

          WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

          total++;

          if(!(total%1000))
             printf_middle("Processing Turn Relations (2): Relations=%"Pindex_t" Deleted=%"Pindex_t" Added=%"Pindex_t,total,deleted,total-relationsx->trnumber+deleted);
         }
      }

    /* Force super nodes on via node and adjacent nodes */

    nodex=LookupNodeX(nodesx,relationx.via,1);
    nodex->flags|=NODE_TURNRSTRCT;
    PutBackNodeX(nodesx,nodex);

    segmentx=FirstSegmentX(segmentsx,relationx.via,1);

    while(segmentx)
      {
       index_t othernode=OtherNode(segmentx,relationx.via);

       nodex=LookupNodeX(nodesx,othernode,1);
       nodex->flags|=NODE_TURNRSTRCT2;
       PutBackNodeX(nodesx,nodex);

       segmentx=NextSegmentX(segmentsx,segmentx,relationx.via);
      }

   endloop: ;
   }

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

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

 printf_last("Processed Turn Relations (2): Relations=%"Pindex_t" Deleted=%"Pindex_t" Added=%"Pindex_t,total,deleted,total-relationsx->trnumber+deleted);

 relationsx->trnumber=total;
}


/*++++++++++++++++++++++++++++++++++++++
  Update the node indexes after geographical sorting.

  RelationsX *relationsx The set of relations to modify.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateTurnRelations(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx)
{
 int trfd;
 index_t i,kept=0;

 /* Print the start message */

 printf_first("Updating Turn Relations (Deleting Pruned): Relations=0");

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->data=MapFile(segmentsx->filename);
#else
 segmentsx->fd=ReOpenFile(segmentsx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Process all of the relations */

 for(i=0;i<relationsx->trnumber;i++)
   {
    TurnRestrictRelX relationx;
    index_t from_node,via_node,to_node;

    ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

    from_node=nodesx->gdata[relationx.from];
    via_node =nodesx->gdata[relationx.via];
    to_node  =nodesx->gdata[relationx.to];

    if(from_node<nodesx->number && via_node<nodesx->number && to_node<nodesx->number)
      {
       SegmentX *segmentx=FirstSegmentX(segmentsx,via_node,1);

       do
         {
          if(OtherNode(segmentx,via_node)==from_node)
             relationx.from=IndexSegmentX(segmentsx,segmentx);

          if(OtherNode(segmentx,via_node)==to_node)
             relationx.to=IndexSegmentX(segmentsx,segmentx);

          segmentx=NextSegmentX(segmentsx,segmentx,via_node);
         }
       while(segmentx);

       relationx.via=via_node;

       WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

       kept++;
      }

    if(!(relationsx->trnumber%1000))
       printf_middle("Updating Turn Relations (Deleting Pruned): Relations=%"Pindex_t,relationsx->trnumber);
   }

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->data=UnmapFile(segmentsx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Updated Turn Relations: Relations=%"Pindex_t" Deleted=%"Pindex_t,kept,relationsx->trnumber-kept);

 relationsx->trnumber=kept;
}


/*++++++++++++++++++++++++++++++++++++++
  Save the relation list to a file.

  RelationsX* relationsx The set of relations to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveRelationList(RelationsX* relationsx,const char *filename)
{
 index_t i;
 int fd;
 RelationsFile relationsfile={0};

 /* Print the start message */

 printf_first("Writing Relations: Turn Relations=0");

 /* Re-open the file read-only */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 /* Write out the relations data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(RelationsFile));

 for(i=0;i<relationsx->trnumber;i++)
   {
    TurnRestrictRelX relationx;
    TurnRelation relation={0};

    ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

    relation.from=relationx.from;
    relation.via=relationx.via;
    relation.to=relationx.to;
    relation.except=relationx.except;

    WriteFile(fd,&relation,sizeof(TurnRelation));

    if(!((i+1)%1000))
       printf_middle("Writing Relations: Turn Relations=%"Pindex_t,i+1);
   }

 /* Write out the header structure */

 relationsfile.trnumber=relationsx->trnumber;

 SeekFile(fd,0);
 WriteFile(fd,&relationsfile,sizeof(RelationsFile));

 CloseFile(fd);

 /* Close the file */

 relationsx->trfd=CloseFile(relationsx->trfd);

 /* Print the final message */

 printf_last("Wrote Relations: Turn Relations=%"Pindex_t,relationsx->trnumber);
}
