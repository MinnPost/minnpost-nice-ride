/***************************************
 Result data type functions.

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


#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "results.h"


/*+ The maximum number of collisions in a bin for the Results 'point' arrays before worrying. +*/
#define MAX_COLLISIONS 32
 

/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *NewResultsList Returns the results list.

  int nbins The initial number of bins in the results array.
  ++++++++++++++++++++++++++++++++++++++*/

Results *NewResultsList(int nbins)
{
 Results *results;

 results=(Results*)malloc(sizeof(Results));

 results->nbins=1;
 results->mask=~0;

 while(nbins>>=1)
   {
    results->mask<<=1;
    results->nbins<<=1;
   }

 results->mask=~results->mask;

 results->number=0;

 results->npoint1=0;

 results->count=(uint8_t*)calloc(results->nbins,sizeof(uint8_t));
 results->point=(Result***)malloc(MAX_COLLISIONS*sizeof(Result**));

 results->ndata1=0;
 results->ndata2=results->nbins;

 results->data=NULL;

 results->start_node=NO_NODE;
 results->prev_segment=NO_SEGMENT;

 results->finish_node=NO_NODE;
 results->last_segment=NO_SEGMENT;

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
 int i;

 for(i=0;i<results->ndata1;i++)
    free(results->data[i]);

 free(results->data);

 for(i=0;i<results->npoint1;i++)
    free(results->point[i]);

 free(results->point);

 free(results->count);

 free(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a new result into the results data structure in the right order.

  Result *InsertResult Returns the result that has been inserted.

  Results *results The results structure to insert into.

  index_t node The node that is to be inserted into the results.

  index_t segment The segment that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *InsertResult(Results *results,index_t node,index_t segment)
{
 Result *result;
 int bin=node&results->mask;

 /* Check if we have hit the limit on the number of collisions per bin */

 if(results->count[bin]>MAX_COLLISIONS && results->count[bin]==results->npoint1)
   {
    int i,j,k;

    results->nbins<<=1;
    results->mask=(results->mask<<1)|1;

    results->count=(uint8_t*)realloc((void*)results->count,results->nbins*sizeof(uint8_t));

    for(i=0;i<results->npoint1;i++)
       results->point[i]=(Result**)realloc((void*)results->point[i],results->nbins*sizeof(Result*));

    for(i=0;i<results->nbins/2;i++)
      {
       int c=results->count[i];

       results->count[i+results->nbins/2]=0;

       for(j=0,k=0;j<c;j++)
         {
          int newbin=results->point[j][i]->node&results->mask;

          if(newbin==i)
            {
             if(k!=j)
                results->point[k][i]=results->point[j][i];
             k++;
            }
          else
            {
             results->point[results->count[newbin]][newbin]=results->point[j][i];

             results->count[newbin]++;
             results->count[i]--;
            }
         }
      }

    bin=node&results->mask;
   }

 /* Check that the arrays have enough space or allocate more. */

 if(results->count[bin]==results->npoint1)
   {
    assert(results->npoint1<255);

    results->npoint1++;

    if(results->npoint1>MAX_COLLISIONS)
       results->point=(Result***)realloc((void*)results->point,results->npoint1*sizeof(Result**));

    results->point[results->npoint1-1]=(Result**)malloc(results->nbins*sizeof(Result*));
   }

 if((results->number%results->ndata2)==0)
   {
    results->ndata1++;

    results->data=(Result**)realloc((void*)results->data,results->ndata1*sizeof(Result*));
    results->data[results->ndata1-1]=(Result*)malloc(results->ndata2*sizeof(Result));
   }

 /* Insert the new entry */

 results->point[results->count[bin]][bin]=&results->data[results->ndata1-1][results->number%results->ndata2];

 results->number++;

 results->count[bin]++;

 /* Initialise the result */

 result=results->point[results->count[bin]-1][bin];

 result->node=node;
 result->segment=segment;

 result->prev=NULL;
 result->next=NULL;

 result->score=0;
 result->sortby=0;

 result->queued=NOT_QUEUED;

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result; search by node only (don't care about the segment but find the shortest).

  Result *FindResult1 Returns the result that has been found.

  Results *results The results structure to search.

  index_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult1(Results *results,index_t node)
{
 int bin=node&results->mask;
 score_t best_score=INF_SCORE;
 Result *best_result=NULL;
 int i;

 for(i=results->count[bin]-1;i>=0;i--)
    if(results->point[i][bin]->node==node && results->point[i][bin]->score<best_score)
      {
       best_score=results->point[i][bin]->score;
       best_result=results->point[i][bin];
      }

 return(best_result);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result; search by node and segment.

  Result *FindResult Returns the result that has been found.

  Results *results The results structure to search.

  index_t node The node that is to be found.

  index_t segment The segment that was used to reach this node.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult(Results *results,index_t node,index_t segment)
{
 int bin=node&results->mask;
 int i;

 for(i=results->count[bin]-1;i>=0;i--)
    if(results->point[i][bin]->segment==segment && results->point[i][bin]->node==node)
       return(results->point[i][bin]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first result from a set of results.

  Result *FirstResult Returns the first result.

  Results *results The set of results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FirstResult(Results *results)
{
 return(&results->data[0][0]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next result from a set of results.

  Result *NextResult Returns the next result.

  Results *results The set of results.

  Result *result The previous result.
  ++++++++++++++++++++++++++++++++++++++*/

Result *NextResult(Results *results,Result *result)
{
 int i,j=0;

 for(i=0;i<results->ndata1;i++)
   {
    j=result-results->data[i];

    if(j>=0 && j<results->ndata2)
       break;
   }

 if(++j>=results->ndata2)
   {i++;j=0;}

 if((i*results->ndata2+j)>=results->number)
    return(NULL);

 return(&results->data[i][j]);
}
