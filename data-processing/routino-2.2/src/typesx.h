/***************************************
 Type definitions for eXtended types.

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


#ifndef TYPESX_H
#define TYPESX_H    /*+ To stop multiple inclusions. +*/


#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


/* Constants and macros for handling them */

/*+ An undefined node ID. +*/
#define NO_NODE_ID     (~(node_t)0)

/*+ An undefined way ID. +*/
#define NO_WAY_ID      (~(way_t)0)

/*+ An undefined relation ID. +*/
#define NO_RELATION_ID (~(relation_t)0)

/*+ The maximum number of segments per node (used to size temporary storage). +*/
#define MAX_SEG_PER_NODE 32


/* Bit mask macro types and functions */

#define BitMask uint32_t

#define AllocBitMask(xx)   (BitMask*)calloc((1+(xx)/32),sizeof(BitMask))

#define SetAllBits0(xx,yy) memset((xx), 0,(1+(yy)/32)*sizeof(BitMask))
#define SetAllBits1(xx,yy) memset((xx),~0,(1+(yy)/32)*sizeof(BitMask))

#define ClearBit(xx,yy)    (xx)[(yy)/32]&=~(((BitMask)1)<<((yy)%32))
#define SetBit(xx,yy)      (xx)[(yy)/32]|= (((BitMask)1)<<((yy)%32))
#define IsBitSet(xx,yy)   ((xx)[(yy)/32]&  (((BitMask)1)<<((yy)%32)))


/* Simple Types */

/*+ A node identifier - must be at least as large as index_t. +*/
typedef uint32_t node_t;

/*+ A way identifier - must be at least as large as index_t. +*/
typedef uint32_t way_t;

/*+ A relation identifier - must be at least as large as index_t. +*/
typedef uint32_t relation_t;


/*+ A printf formatting string for a node_t type (this should match the node_t definition above). +*/
#define Pnode_t PRIu32          /* PRIu32 and PRIu64 are defined in intypes.h */

/*+ A printf formatting string for a way_t type (this should match the way_t definition above). +*/
#define Pway_t PRIu32           /* PRIu32 and PRIu64 are defined in intypes.h */

/*+ A printf formatting string for a relation_t type (this should match the relation_t definition above). +*/
#define Prelation_t PRIu32      /* PRIu32 and PRIu64 are defined in intypes.h */


/* Enumerated types */

/*+ Turn restrictions. +*/
typedef enum _TurnRestriction
 {
  TurnRestrict_None              =0,
  TurnRestrict_no_right_turn,
  TurnRestrict_no_left_turn,
  TurnRestrict_no_u_turn,
  TurnRestrict_no_straight_on,
  TurnRestrict_only_right_turn,
  TurnRestrict_only_left_turn,
  TurnRestrict_only_straight_on
 }
 TurnRestriction;


/* Data structures */

typedef struct _NodeX NodeX;

typedef struct _NodesX NodesX;

typedef struct _SegmentX SegmentX;

typedef struct _SegmentsX SegmentsX;

typedef struct _WayX WayX;

typedef struct _WaysX WaysX;

typedef struct _RouteRelX RouteRelX;

typedef struct _TurnRestrictRelX TurnRestrictRelX;

typedef struct _RelationsX RelationsX;


#endif /* TYPESX_H */
