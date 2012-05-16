/***************************************
 Node data type functions.

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
#include <math.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "files.h"
#include "profiles.h"


/* Local functions */

static int valid_segment_for_profile(Ways *ways,Segment *segment,Profile *profile);


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes *LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 Nodes *nodes;
#if SLIM
 size_t sizeoffsets;
 int i;
#endif

 nodes=(Nodes*)malloc(sizeof(Nodes));

#if !SLIM

 nodes->data=MapFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 nodes->file=*((NodesFile*)nodes->data);

 /* Set the pointers in the Nodes structure. */

 nodes->offsets=(index_t*)(nodes->data+sizeof(NodesFile));
 nodes->nodes  =(Node*   )(nodes->data+sizeof(NodesFile)+(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t));

#else

 nodes->fd=ReOpenFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 ReadFile(nodes->fd,&nodes->file,sizeof(NodesFile));

 sizeoffsets=(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t);

 nodes->offsets=(index_t*)malloc(sizeoffsets);

 ReadFile(nodes->fd,nodes->offsets,sizeoffsets);

 nodes->nodesoffset=sizeof(NodesFile)+sizeoffsets;

 for(i=0;i<sizeof(nodes->cached)/sizeof(nodes->cached[0]);i++)
    nodes->incache[i]=NO_NODE;

#endif

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest node given its latitude, longitude and the profile of the
  mode of transport that must be able to move to/from this node.

  index_t FindClosestNode Returns the closest node.

  Nodes *nodes The set of nodes to search.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look from the specified coordinates.

  Profile *profile The profile of the mode of transport.

  distance_t *bestdist Returns the distance to the best node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestNode(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn=NO_NODE;
 distance_t bestd=INF_DISTANCE;

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    ll_bin_t latb,lonb;
    ll_bin2_t llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          llbin=lonb*nodes->file.latbins+latb;

          /* Check if this grid square has any hope of being close enough */

          if(delta>0)
            {
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb));
             double lat2=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb+1));
             double lon2=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb+1));

             if(latb==latbin)
               {
                distance_t dist1=Distance(latitude,lon1,latitude,longitude);
                distance_t dist2=Distance(latitude,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else if(lonb==lonbin)
               {
                distance_t dist1=Distance(lat1,longitude,latitude,longitude);
                distance_t dist2=Distance(lat2,longitude,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else
               {
                distance_t dist1=Distance(lat1,lon1,latitude,longitude);
                distance_t dist2=Distance(lat2,lon1,latitude,longitude);
                distance_t dist3=Distance(lat2,lon2,latitude,longitude);
                distance_t dist4=Distance(lat1,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance && dist3>distance && dist4>distance)
                   continue;
               }
            }

          /* Check every node in this grid square. */

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *node=LookupNode(nodes,i,3);
             double lat=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(node->latoffset));
             double lon=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(node->lonoffset));

             distance_t dist=Distance(lat,lon,latitude,longitude);

             if(dist<distance)
               {
                Segment *segment;

                /* Check that at least one segment is valid for the profile */

                segment=FirstSegment(segments,node,1);

                do
                  {
                   if(IsNormalSegment(segment) && valid_segment_for_profile(ways,segment,profile))
                     {
                      bestn=i;
                      bestd=distance=dist;

                      break;
                     }

                   segment=NextSegment(segments,segment,i);
                  }
                while(segment);
               }
            }

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 return(bestn);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest point on the closest segment given its latitude, longitude
  and the profile of the mode of transport that must be able to move along this
  segment.

  index_t FindClosestSegment Returns the closest segment index.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to search.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look from the specified coordinates.

  Profile *profile The profile of the mode of transport.

  distance_t *bestdist Returns the distance to the closest point on the best segment.

  index_t *bestnode1 Returns the index of the node at one end of the closest segment.

  index_t *bestnode2 Returns the index of the node at the other end of the closest segment.

  distance_t *bestdist1 Returns the distance along the segment to the node at one end.

  distance_t *bestdist2 Returns the distance along the segment to the node at the other end.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestSegment(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                           distance_t distance,Profile *profile, distance_t *bestdist,
                           index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn1=NO_NODE,bestn2=NO_NODE;
 distance_t bestd=INF_DISTANCE,bestd1=INF_DISTANCE,bestd2=INF_DISTANCE;
 index_t    bests=NO_SEGMENT;

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    ll_bin_t latb,lonb;
    ll_bin2_t llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          llbin=lonb*nodes->file.latbins+latb;

          /* Check if this grid square has any hope of being close enough */

          if(delta>0)
            {
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb));
             double lat2=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb+1));
             double lon2=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb+1));

             if(latb==latbin)
               {
                distance_t dist1=Distance(latitude,lon1,latitude,longitude);
                distance_t dist2=Distance(latitude,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else if(lonb==lonbin)
               {
                distance_t dist1=Distance(lat1,longitude,latitude,longitude);
                distance_t dist2=Distance(lat2,longitude,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else
               {
                distance_t dist1=Distance(lat1,lon1,latitude,longitude);
                distance_t dist2=Distance(lat2,lon1,latitude,longitude);
                distance_t dist3=Distance(lat2,lon2,latitude,longitude);
                distance_t dist4=Distance(lat1,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance && dist3>distance && dist4>distance)
                   continue;
               }
            }

          /* Check every node in this grid square. */

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *node=LookupNode(nodes,i,3);
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(node->latoffset));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(node->lonoffset));
             distance_t dist1;

             dist1=Distance(lat1,lon1,latitude,longitude);

             if(dist1<distance)
               {
                Segment *segment;

                /* Check each segment for closeness and if valid for the profile */

                segment=FirstSegment(segments,node,1);

                do
                  {
                   if(IsNormalSegment(segment) && valid_segment_for_profile(ways,segment,profile))
                     {
                      distance_t dist2,dist3;
                      double lat2,lon2,dist3a,dist3b,distp;

                      GetLatLong(nodes,OtherNode(segment,i),&lat2,&lon2);

                      dist2=Distance(lat2,lon2,latitude,longitude);

                      dist3=Distance(lat1,lon1,lat2,lon2);

                      /* Use law of cosines (assume flat Earth) */

                      dist3a=((double)dist1*(double)dist1-(double)dist2*(double)dist2+(double)dist3*(double)dist3)/(2.0*(double)dist3);
                      dist3b=(double)dist3-dist3a;

                      if((dist1+dist2)<dist3)
                        {
                         distp=0;
                        }
                      else if(dist3a>=0 && dist3b>=0)
                         distp=sqrt((double)dist1*(double)dist1-dist3a*dist3a);
                      else if(dist3a>0)
                        {
                         distp=dist2;
                         dist3a=dist3;
                         dist3b=0;
                        }
                      else /* if(dist3b>0) */
                        {
                         distp=dist1;
                         dist3a=0;
                         dist3b=dist3;
                        }

                      if(distp<(double)bestd)
                        {
                         bests=IndexSegment(segments,segment);

                         if(segment->node1==i)
                           {
                            bestn1=i;
                            bestn2=OtherNode(segment,i);
                            bestd1=(distance_t)dist3a;
                            bestd2=(distance_t)dist3b;
                           }
                         else
                           {
                            bestn1=OtherNode(segment,i);
                            bestn2=i;
                            bestd1=(distance_t)dist3b;
                            bestd2=(distance_t)dist3a;
                           }

                         bestd=(distance_t)distp;
                        }
                     }

                   segment=NextSegment(segments,segment,i);
                  }
                while(segment);
               }

            } /* dist1 < distance */

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 *bestnode1=bestn1;
 *bestnode2=bestn2;
 *bestdist1=bestd1;
 *bestdist2=bestd2;

 return(bests);
}


/*++++++++++++++++++++++++++++++++++++++
  Check if the transport defined by the profile is allowed on the segment.

  int valid_segment_for_profile Return 1 if it is or 0 if not.

  Ways *ways The set of ways to use.

  Segment *segment The segment to check.

  Profile *profile The profile to check.
  ++++++++++++++++++++++++++++++++++++++*/

static int valid_segment_for_profile(Ways *ways,Segment *segment,Profile *profile)
{
 Way *way=LookupWay(ways,segment->way,1);
 score_t segment_pref;
 int i;

 /* mode of transport must be allowed on the highway */
 if(!(way->allow&profile->allow))
    return(0);

 /* must obey weight restriction (if exists) */
 if(way->weight && way->weight<profile->weight)
    return(0);

 /* must obey height/width/length restriction (if exists) */
 if((way->height && way->height<profile->height) ||
    (way->width  && way->width <profile->width ) ||
    (way->length && way->length<profile->length))
    return(0);

 segment_pref=profile->highway[HIGHWAY(way->type)];

 for(i=1;i<Property_Count;i++)
    if(ways->file.props & PROPERTIES(i))
      {
       if(way->props & PROPERTIES(i))
          segment_pref*=profile->props_yes[i];
       else
          segment_pref*=profile->props_no[i];
      }

 /* profile preferences must allow this highway */
 if(segment_pref==0)
    return(0);

 /* Must be OK */
 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the latitude and longitude associated with a node.

  Nodes *nodes The set of nodes to use.

  index_t index The node index.

  double *latitude Returns the latitude.

  double *longitude Returns the logitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetLatLong(Nodes *nodes,index_t index,double *latitude,double *longitude)
{
 Node *node=LookupNode(nodes,index,4);
 ll_bin_t latbin=-1,lonbin=-1;
 ll_bin_t start,end,mid;
 index_t offset;

 /* Binary search - search key nearest match below is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  A lower bound match is wanted we can set end=mid-1 or
  *  # <- mid    |  start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 /* Search for longitude */

 start=0;
 end=nodes->file.lonbins-1;

 do
   {
    mid=(start+end)/2;                  /* Choose mid point */

    offset=LookupNodeOffset(nodes,nodes->file.latbins*mid);

    if(offset<index)                    /* Mid point is too low for an exact match but could be lower bound */
       start=mid;
    else if(offset>index)               /* Mid point is too high */
       end=mid?(mid-1):mid;
    else                                /* Mid point is correct */
      {lonbin=mid;break;}
   }
 while((end-start)>1);

 if(lonbin==-1)
   {
    offset=LookupNodeOffset(nodes,nodes->file.latbins*end);

    if(offset>index)
       lonbin=start;
    else
       lonbin=end;
   }

 while(lonbin<nodes->file.lonbins && 
       LookupNodeOffset(nodes,lonbin*nodes->file.latbins)==LookupNodeOffset(nodes,(lonbin+1)*nodes->file.latbins))
    lonbin++;

 /* Search for latitude */

 start=0;
 end=nodes->file.latbins-1;

 do
   {
    mid=(start+end)/2;                  /* Choose mid point */

    offset=LookupNodeOffset(nodes,lonbin*nodes->file.latbins+mid);

    if(offset<index)                    /* Mid point is too low for an exact match but could be lower bound */
       start=mid;
    else if(offset>index)               /* Mid point is too high */
       end=mid?(mid-1):mid;
    else                                /* Mid point is correct */
      {latbin=mid;break;}
   }
 while((end-start)>1);

 if(latbin==-1)
   {
    offset=LookupNodeOffset(nodes,lonbin*nodes->file.latbins+end);

    if(offset>index)
       latbin=start;
    else
       latbin=end;
   }

 while(latbin<nodes->file.latbins &&
       LookupNodeOffset(nodes,lonbin*nodes->file.latbins+latbin)==LookupNodeOffset(nodes,lonbin*nodes->file.latbins+latbin+1))
    latbin++;

 /* Return the values */

 *latitude =latlong_to_radians(bin_to_latlong(nodes->file.latzero+latbin)+off_to_latlong(node->latoffset));
 *longitude=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonbin)+off_to_latlong(node->lonoffset));
}
