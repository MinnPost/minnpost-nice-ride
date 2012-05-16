/***************************************
 Header file for fake node and segment function prototypes

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


#ifndef FAKES_H
#define FAKES_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Macros */

/*+ Return true if this is a fake node. +*/
#define IsFakeNode(xxx)    ((xxx)>=NODE_FAKE && (xxx)!=NO_NODE)

/*+ Return true if this is a fake segment. +*/
#define IsFakeSegment(xxx) ((xxx)>=SEGMENT_FAKE && (xxx)!=NO_SEGMENT)


/* Functions in fakes.c */

index_t CreateFakes(Nodes *nodes,Segments *segments,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2);

void GetFakeLatLong(index_t fakenode, double *latitude,double *longitude);

Segment *FirstFakeSegment(index_t fakenode);
Segment *NextFakeSegment(Segment *fakesegment,index_t fakenode);
Segment *ExtraFakeSegment(index_t realnode,index_t fakenode);

Segment *LookupFakeSegment(index_t index);
index_t IndexFakeSegment(Segment *fakesegment);
index_t IndexRealSegment(index_t fakesegment);

int IsFakeUTurn(index_t fakesegment1,index_t fakesegment2);

#endif /* FAKES_H */
