/***************************************
 Header for super-node and super-segment pruning functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2011-2012 Andrew M. Bishop

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


#ifndef PRUNEX_H
#define PRUNEX_H    /*+ To stop multiple inclusions. +*/

#include "types.h"

#include "typesx.h"


/* Functions in prunex.c */

void StartPruning(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx);
void FinishPruning(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx);

void PruneIsolatedRegions(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,distance_t minimum);

void PruneShortSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,distance_t minimum);

void PruneStraightHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,distance_t maximum);


#endif /* PRUNEX_H */
