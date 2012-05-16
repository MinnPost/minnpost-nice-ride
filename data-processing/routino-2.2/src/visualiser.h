/***************************************
 Header file for visualiser functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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


#ifndef VISUALISER_H
#define VISUALISER_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Functions in visualiser.c */

void OutputJunctions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);

void OutputSuper(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);

void OutputOneway(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);
void OutputTurnRestrictions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);

void OutputSpeedLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);

void OutputWeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);

void OutputHeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);
void OutputWidthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);
void OutputLengthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax);


#endif /* VISUALISER_H */
