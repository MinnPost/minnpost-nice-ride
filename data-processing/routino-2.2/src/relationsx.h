/***************************************
 A header file for the extended Relations structure.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2011 Andrew M. Bishop

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


#ifndef RELATIONSX_H
#define RELATIONSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "typesx.h"


/* Data structures */


/*+ An extended structure containing a single route relation. +*/
struct _RouteRelX
{
 relation_t   id;              /*+ The relation identifier. +*/

 transports_t routes;          /*+ The types of transports that that this relation is for. +*/
};


/*+ An extended structure containing a single turn restriction relation. +*/
struct _TurnRestrictRelX
{
 relation_t      id;           /*+ The relation identifier. +*/

 way_t           from;         /*+ The id of the starting way; initially the OSM value, later the NodeX index then the SegmentX index. +*/
 node_t          via;          /*+ The id of the via node; initially the OSM value, later the NodeX index. +*/
 way_t           to;           /*+ The id of the ending way; initially the OSM value, later the NodeX index then the SegmentX index. +*/

 TurnRestriction restriction;  /*+ The type of restriction. +*/
 transports_t    except;       /*+ The types of transports that that this relation does not apply to. +*/
};


/*+ A structure containing a set of relations. +*/
struct _RelationsX
{
 /* Route relations */

 char      *rfilename;         /*+ The name of the temporary file (for the RouteRelX). +*/
 int        rfd;               /*+ The file descriptor of the temporary file (for the RouteRelX). +*/

 index_t    rnumber;           /*+ The number of extended route relations. +*/

 /* Turn restriction relations */

 char      *trfilename;        /*+ The name of the temporary file (for the TurnRestrictRelX). +*/
 int        trfd;              /*+ The file descriptor of the temporary file (for the TurnRestrictRelX). +*/

 index_t    trnumber;          /*+ The number of extended turn restriction relations. +*/
};


/* Functions in relationsx.c */

RelationsX *NewRelationList(int append);
void FreeRelationList(RelationsX *relationsx,int keep);

void AppendRouteRelation(RelationsX* relationsx,relation_t id,
                         transports_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations);

void AppendTurnRestrictRelation(RelationsX* relationsx,relation_t id,
                                way_t from,way_t to,node_t via,
                                TurnRestriction restriction,transports_t except);

void SortRelationList(RelationsX *relationsx);

void SortTurnRelationList(RelationsX* relationsx);

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx);

void ProcessTurnRelations1(RelationsX *relationsx,NodesX *nodesx,WaysX *waysx);
void ProcessTurnRelations2(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx);

void UpdateTurnRelations(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx);

void SaveRelationList(RelationsX* relationsx,const char *filename);


#endif /* RELATIONSX_H */
