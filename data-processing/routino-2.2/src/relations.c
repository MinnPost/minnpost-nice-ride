/***************************************
 Relation data type functions.

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

#include "types.h"
#include "relations.h"
#include "fakes.h"

#include "files.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a relation list from a file.

  Relations *LoadRelationList Returns the relation list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Relations *LoadRelationList(const char *filename)
{
 Relations *relations;
#if SLIM
 int i;
#endif

 relations=(Relations*)malloc(sizeof(Relations));

#if !SLIM

 relations->data=MapFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 relations->file=*((RelationsFile*)relations->data);

 /* Set the pointers in the Relations structure. */

 relations->turnrelations=(TurnRelation*)(relations->data+sizeof(RelationsFile));

#else

 relations->fd=ReOpenFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 ReadFile(relations->fd,&relations->file,sizeof(RelationsFile));

 relations->troffset=sizeof(RelationsFile);

 for(i=0;i<sizeof(relations->cached)/sizeof(relations->cached[0]);i++)
    relations->incache[i]=NO_RELATION;

#endif

 if(relations->file.trnumber>0)
   {
    TurnRelation *relation;

    relation=LookupTurnRelation(relations,0,1);

    relations->via_start =relation->via;

    relation=LookupTurnRelation(relations,relations->file.trnumber-1,1);

    relations->via_end =relation->via;
   }

 return(relations);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first turn relation in the file whose 'via' matches a specific node.

  index_t FindFirstTurnRelation1 Returns the index of the first turn relation matching.

  Relations *relations The set of relations to use.

  index_t via The node that the route is going via.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindFirstTurnRelation1(Relations *relations,index_t via)
{
 TurnRelation *relation;
 index_t start=0;
 index_t end=relations->file.trnumber-1;
 index_t mid;
 index_t match=-1;

 /* Binary search - search key any exact match is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end matches (but may not be the first).
  */

 do
   {
    mid=(start+end)/2;              /* Choose mid point */

    relation=LookupTurnRelation(relations,mid,1);

    if(relation->via<via)           /* Mid point is too low for 'via' */
       start=mid+1;
    else if(relation->via>via)      /* Mid point is too high for 'via' */
       end=mid?(mid-1):mid;
    else                            /* Mid point is correct for 'from' */
      {
       match=mid;
       break;
      }
   }
 while((end-start)>1);

 if(match==-1)                      /* Check if start matches */
   {
    relation=LookupTurnRelation(relations,start,1);

    if(relation->via==via)
       match=start;
   }

 if(match==-1)                      /* Check if end matches */
   {
    relation=LookupTurnRelation(relations,end,1);

    if(relation->via==via)
       match=end;
   }

 if(match==-1)
    return(NO_RELATION);

 while(match>0)                     /* Search backwards for the first match */
   {
    relation=LookupTurnRelation(relations,match-1,1);

    if(relation->via==via)
       match--;
    else
       break;
   }

 return(match);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next turn relation in the file whose 'via' matches a specific node.

  index_t FindNextTurnRelation1 Returns the index of the next turn relation matching.

  Relations *relations The set of relations to use.

  index_t current The current index of a relation that matches.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindNextTurnRelation1(Relations *relations,index_t current)
{
 TurnRelation *relation;
 index_t via;

 relation=LookupTurnRelation(relations,current,1);

 via=relation->via;

 current++;

 if(current==relations->file.trnumber)
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,current,1);

 if(relation->via==via)
    return(current);
 else
    return(NO_RELATION);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first turn relation in the file whose 'via' and 'from' match a specific node and segment.

  index_t FindFirstTurnRelation2 Returns the index of the first turn relation matching.

  Relations *relations The set of relations to use.

  index_t via The node that the route is going via.

  index_t from The segment that the route is coming from.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindFirstTurnRelation2(Relations *relations,index_t via,index_t from)
{
 TurnRelation *relation;
 index_t start=0;
 index_t end=relations->file.trnumber-1;
 index_t mid;
 index_t match=-1;

 if(IsFakeSegment(from))
    from=IndexRealSegment(from);

 /* Binary search - search key first match is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end matches (but may not be the first).
  */

 do
   {
    mid=(start+end)/2;              /* Choose mid point */

    relation=LookupTurnRelation(relations,mid,1);

    if(relation->via<via)           /* Mid point is too low for 'via' */
       start=mid+1;
    else if(relation->via>via)      /* Mid point is too high for 'via' */
       end=mid?(mid-1):mid;
    else                            /* Mid point is correct for 'via' */
      {
       if(relation->from<from)      /* Mid point is too low for 'from' */
          start=mid+1;
       else if(relation->from>from) /* Mid point is too high for 'from' */
          end=mid?(mid-1):mid;
       else                         /* Mid point is correct for 'from' */
         {
          match=mid;
          break;
         }
      }
   }
 while((end-start)>1);

 if(match==-1)                      /* Check if start matches */
   {
    relation=LookupTurnRelation(relations,start,1);

    if(relation->via==via && relation->from==from)
       match=start;
   }

 if(match==-1)                      /* Check if end matches */
   {
    relation=LookupTurnRelation(relations,end,1);

    if(relation->via==via && relation->from==from)
       match=end;
   }

 if(match==-1)
    return(NO_RELATION);

 while(match>0)                     /* Search backwards for the first match */
   {
    relation=LookupTurnRelation(relations,match-1,1);

    if(relation->via==via && relation->from==from)
       match--;
    else
       break;
   }

 return(match);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next turn relation in the file whose 'via' and 'from' match a specific node and segment.

  index_t FindNextTurnRelation2 Returns the index of the next turn relation matching.

  Relations *relations The set of relations to use.

  index_t current The current index of a relation that matches.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindNextTurnRelation2(Relations *relations,index_t current)
{
 TurnRelation *relation;
 index_t via,from;

 relation=LookupTurnRelation(relations,current,1);

 via=relation->via;
 from=relation->from;

 current++;

 if(current==relations->file.trnumber)
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,current,1);

 if(relation->via==via && relation->from==from)
    return(current);
 else
    return(NO_RELATION);
}


/*++++++++++++++++++++++++++++++++++++++
  Determine if a turn is allowed between the nodes 'from', 'via' and 'to' for a particular transport type.

  int IsTurnAllowed Return 1 if the turn is allowed or 0 if not.

  Relations *relations The set of relations to use.

  index_t index The index of the first turn relation containing 'via' and 'from'.

  index_t via The via node.

  index_t from The from segment.

  index_t to The to segment.

  transports_t transport The type of transport that is being routed.
  ++++++++++++++++++++++++++++++++++++++*/

int IsTurnAllowed(Relations *relations,index_t index,index_t via,index_t from,index_t to,transports_t transport)
{
 if(IsFakeSegment(from))
    from=IndexRealSegment(from);

 if(IsFakeSegment(to))
    to=IndexRealSegment(to);

 while(index<relations->file.trnumber)
   {
    TurnRelation *relation=LookupTurnRelation(relations,index,1);

    if(relation->via!=via)
       return(1);

    if(relation->from!=from)
       return(1);

    if(relation->to>to)
       return(1);

    if(relation->to==to)
       if(!(relation->except & transport))
          return(0);

    index++;
   }

 return(1);
}
