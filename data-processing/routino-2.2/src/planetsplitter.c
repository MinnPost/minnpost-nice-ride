/***************************************
 OSM planet file splitter.

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "ways.h"

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"
#include "superx.h"
#include "prunex.h"

#include "files.h"
#include "logging.h"
#include "functions.h"
#include "osmparser.h"
#include "tagging.h"


/* Global variables */

/*+ The name of the temporary directory. +*/
char *option_tmpdirname=NULL;

/*+ The amount of RAM to use for filesorting. +*/
size_t option_filesort_ramsize=0;


/* Local functions */

static void print_usage(int detail,const char *argerr,const char *err);


/*++++++++++++++++++++++++++++++++++++++
  The main program for the planetsplitter.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 NodesX     *Nodes;
 SegmentsX  *Segments,*SuperSegments=NULL,*MergedSegments=NULL;
 WaysX      *Ways;
 RelationsX *Relations;
 int         iteration=0,quit=0;
 int         max_iterations=5;
 char       *dirname=NULL,*prefix=NULL,*tagging=NULL,*errorlog=NULL;
 int         option_parse_only=0,option_process_only=0;
 int         option_filenames=0;
 int         option_prune_isolated=500,option_prune_short=5,option_prune_straight=3;
 int         arg;

 /* Parse the command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--help"))
       print_usage(1,NULL,NULL);
    else if(!strncmp(argv[arg],"--sort-ram-size=",16))
       option_filesort_ramsize=atoi(&argv[arg][16]);
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--tmpdir=",9))
       option_tmpdirname=&argv[arg][9];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strcmp(argv[arg],"--parse-only"))
       option_parse_only=1;
    else if(!strcmp(argv[arg],"--process-only"))
       option_process_only=1;
    else if(!strcmp(argv[arg],"--loggable"))
       option_loggable=1;
    else if(!strcmp(argv[arg],"--errorlog"))
       errorlog="error.log";
    else if(!strncmp(argv[arg],"--errorlog=",11))
       errorlog=&argv[arg][11];
    else if(!strncmp(argv[arg],"--max-iterations=",17))
       max_iterations=atoi(&argv[arg][17]);
    else if(!strncmp(argv[arg],"--tagging=",10))
       tagging=&argv[arg][10];
    else if(!strncmp(argv[arg],"--prune",7))
      {
       if(!strcmp(&argv[arg][7],"-none"))
          option_prune_isolated=option_prune_short=option_prune_straight=0;
       else if(!strncmp(&argv[arg][7],"-isolated=",10))
          option_prune_isolated=atoi(&argv[arg][17]);
       else if(!strncmp(&argv[arg][7],"-short=",7))
          option_prune_short=atoi(&argv[arg][14]);
       else if(!strncmp(&argv[arg][7],"-straight=",10))
          option_prune_straight=atoi(&argv[arg][17]);
       else
          print_usage(0,argv[arg],NULL);
      }
    else if(argv[arg][0]=='-' && argv[arg][1]=='-')
       print_usage(0,argv[arg],NULL);
    else
       option_filenames++;
   }

 /* Check the specified command line options */

 if(option_parse_only && option_process_only)
    print_usage(0,NULL,"Cannot use '--parse-only' and '--process-only' at the same time.");

 if(option_filenames && option_process_only)
    print_usage(0,NULL,"Cannot use '--process-only' and filenames at the same time.");

 if(!option_filesort_ramsize)
   {
#if SLIM
    option_filesort_ramsize=64*1024*1024;
#else
    option_filesort_ramsize=256*1024*1024;
#endif
   }
 else
    option_filesort_ramsize*=1024*1024;

 if(!option_tmpdirname)
   {
    if(!dirname)
       option_tmpdirname=".";
    else
       option_tmpdirname=dirname;
   }

 if(tagging)
   {
    if(!ExistsFile(tagging))
      {
       fprintf(stderr,"Error: The '--tagging' option specifies a file that does not exist.\n");
       return(1);
      }
   }
 else
   {
    if(ExistsFile(FileName(dirname,prefix,"tagging.xml")))
       tagging=FileName(dirname,prefix,"tagging.xml");
    else if(ExistsFile(FileName(DATADIR,NULL,"tagging.xml")))
       tagging=FileName(DATADIR,NULL,"tagging.xml");
    else
      {
       fprintf(stderr,"Error: The '--tagging' option was not used and the default 'tagging.xml' does not exist.\n");
       return(1);
      }
   }

 if(ParseXMLTaggingRules(tagging))
   {
    fprintf(stderr,"Error: Cannot read the tagging rules in the file '%s'.\n",tagging);
    return(1);
   }

 /* Create new node, segment, way and relation variables */

 Nodes=NewNodeList(option_parse_only||option_process_only);

 Segments=NewSegmentList(option_parse_only||option_process_only);

 Ways=NewWayList(option_parse_only||option_process_only);

 Relations=NewRelationList(option_parse_only||option_process_only);

 /* Create the error log file */

 if(errorlog)
    open_errorlog(FileName(dirname,prefix,errorlog),option_parse_only||option_process_only);

 /* Parse the file */

 if(option_filenames)
   {
    for(arg=1;arg<argc;arg++)
      {
       FILE *file;

       if(argv[arg][0]=='-' && argv[arg][1]=='-')
          continue;

       file=fopen(argv[arg],"rb");

       if(!file)
         {
          fprintf(stderr,"Cannot open file '%s' for reading [%s].\n",argv[arg],strerror(errno));
          exit(EXIT_FAILURE);
         }

       printf("\nParse OSM Data [%s]\n==============\n\n",argv[arg]);
       fflush(stdout);

       if(ParseOSM(file,Nodes,Segments,Ways,Relations))
          exit(EXIT_FAILURE);

       fclose(file);
      }
   }
 else if(!option_process_only)
   {
    printf("\nParse OSM Data\n==============\n\n");
    fflush(stdout);

    if(ParseOSM(stdin,Nodes,Segments,Ways,Relations))
       exit(EXIT_FAILURE);
   }

 if(option_parse_only)
   {
    FreeNodeList(Nodes,1);
    FreeSegmentList(Segments,1);
    FreeWayList(Ways,1);
    FreeRelationList(Relations,1);

    return(0);
   }

 DeleteXMLTaggingRules();

 /* Process the data */

 printf("\nProcess OSM Data\n================\n\n");
 fflush(stdout);

 /* Sort the nodes, segments, ways and relations */

 SortNodeList(Nodes);

 SortSegmentList(Segments,0);

 SortWayList(Ways);

 SortRelationList(Relations);

 /* Remove bad segments (must be after sorting the nodes and segments) */

 RemoveBadSegments(Nodes,Segments);

 /* Remove non-highway nodes (must be after removing the bad segments) */

 RemoveNonHighwayNodes(Nodes,Segments);

 /* Process the route relations and first part of turn relations (must be before compacting the ways) */

 ProcessRouteRelations(Relations,Ways);

 ProcessTurnRelations1(Relations,Nodes,Ways);

 /* Compact the ways (must be before measuring the segments) */

 CompactWayList(Ways);

 /* Measure the segments and replace node/way id with index (must be after removing non-highway nodes) */

 MeasureSegments(Segments,Nodes,Ways);

 /* Index the segments */

 IndexSegments(Segments,Nodes);

 /* Convert the turn relations from ways into nodes */

 ProcessTurnRelations2(Relations,Nodes,Segments,Ways);

 /* Prune unwanted nodes/segments. */

 if(option_prune_straight || option_prune_isolated || option_prune_short)
   {
    StartPruning(Nodes,Segments,Ways);

    if(option_prune_straight)
       PruneStraightHighwayNodes(Nodes,Segments,Ways,option_prune_straight);

    if(option_prune_isolated)
       PruneIsolatedRegions(Nodes,Segments,Ways,option_prune_isolated);

    if(option_prune_short)
       PruneShortSegments(Nodes,Segments,Ways,option_prune_short);

    FinishPruning(Nodes,Segments,Ways);
   }

 /* Repeated iteration on Super-Nodes and Super-Segments */

 do
   {
    int nsuper;

    printf("\nProcess Super-Data (iteration %d)\n================================%s\n\n",iteration,iteration>9?"=":"");
    fflush(stdout);

    if(iteration==0)
      {
       /* Select the super-nodes */

       ChooseSuperNodes(Nodes,Segments,Ways);

       /* Select the super-segments */

       SuperSegments=CreateSuperSegments(Nodes,Segments,Ways);

       nsuper=Segments->number;
      }
    else
      {
       SegmentsX *SuperSegments2;

       /* Select the super-nodes */

       ChooseSuperNodes(Nodes,SuperSegments,Ways);

       /* Select the super-segments */

       SuperSegments2=CreateSuperSegments(Nodes,SuperSegments,Ways);

       nsuper=SuperSegments->number;

       FreeSegmentList(SuperSegments,0);

       SuperSegments=SuperSegments2;
      }

    /* Sort the super-segments */

    SortSegmentList(SuperSegments,0);

    /* Remove duplicated super-segments */

    DeduplicateSegments(SuperSegments,Nodes,Ways);

    /* Index the segments */

    IndexSegments(SuperSegments,Nodes);

    /* Check for end condition */

    if(SuperSegments->number==nsuper)
       quit=1;

    iteration++;

    if(iteration>max_iterations)
       quit=1;
   }
 while(!quit);

 /* Combine the super-segments */

 printf("\nCombine Segments and Super-Segments\n===================================\n\n");
 fflush(stdout);

 /* Merge the super-segments */

 MergedSegments=MergeSuperSegments(Segments,SuperSegments);

 FreeSegmentList(Segments,0);

 FreeSegmentList(SuperSegments,0);

 Segments=MergedSegments;

 /* Sort and re-index the segments */

 SortSegmentList(Segments,0);

 IndexSegments(Segments,Nodes);

 /* Cross reference the nodes and segments */

 printf("\nCross-Reference Nodes and Segments\n==================================\n\n");
 fflush(stdout);

 /* Sort the nodes geographically and update the segment indexes accordingly */

 SortNodeListGeographically(Nodes);

 UpdateSegments(Segments,Nodes,Ways);

 /* Sort the segments geographically and re-index them */

 SortSegmentList(Segments,0);

 IndexSegments(Segments,Nodes);

 /* Update the nodes */

 UpdateNodes(Nodes,Segments);

 /* Fix the turn relations after sorting nodes geographically */

 UpdateTurnRelations(Relations,Nodes,Segments);

 SortTurnRelationList(Relations);

 /* Output the results */

 printf("\nWrite Out Database Files\n========================\n\n");
 fflush(stdout);

 /* Write out the nodes */

 SaveNodeList(Nodes,FileName(dirname,prefix,"nodes.mem"));

 FreeNodeList(Nodes,0);

 /* Write out the segments */

 SaveSegmentList(Segments,FileName(dirname,prefix,"segments.mem"));

 FreeSegmentList(Segments,0);

 /* Write out the ways */

 SaveWayList(Ways,FileName(dirname,prefix,"ways.mem"));

 FreeWayList(Ways,0);

 /* Write out the relations */

 SaveRelationList(Relations,FileName(dirname,prefix,"relations.mem"));

 FreeRelationList(Relations,0);

 /* Close the error log file */

 if(errorlog)
    close_errorlog();

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the usage information.

  int detail The level of detail to use - 0 = low, 1 = high.

  const char *argerr The argument that gave the error (if there is one).

  const char *err Other error message (if there is one).
  ++++++++++++++++++++++++++++++++++++++*/

static void print_usage(int detail,const char *argerr,const char *err)
{
 fprintf(stderr,
         "Usage: planetsplitter [--help]\n"
         "                      [--dir=<dirname>] [--prefix=<name>]\n"
         "                      [--sort-ram-size=<size>]\n"
         "                      [--tmpdir=<dirname>]\n"
         "                      [--tagging=<filename>]\n"
         "                      [--loggable] [--errorlog[=<name>]]\n"
         "                      [--parse-only | --process-only]\n"
         "                      [--max-iterations=<number>]\n"
         "                      [--prune-none]\n"
         "                      [--prune-isolated=<len>]\n"
         "                      [--prune-short=<len>]\n"
         "                      [--prune-straight=<len>]\n"
         "                      [<filename.osm> ...]\n");

 if(argerr)
    fprintf(stderr,
            "\n"
            "Error with command line parameter: %s\n",argerr);

 if(err)
    fprintf(stderr,
            "\n"
            "Error: %s\n",err);

 if(detail)
    fprintf(stderr,
            "\n"
            "--help                    Prints this information.\n"
            "\n"
            "--dir=<dirname>           The directory containing the routing database.\n"
            "--prefix=<name>           The filename prefix for the routing database.\n"
            "\n"
            "--sort-ram-size=<size>    The amount of RAM (in MB) to use for data sorting\n"
#if SLIM
            "                          (defaults to 64MB otherwise.)\n"
#else
            "                          (defaults to 256MB otherwise.)\n"
#endif
            "--tmpdir=<dirname>        The directory name for temporary files.\n"
            "                          (defaults to the '--dir' option directory.)\n"
            "\n"
            "--tagging=<filename>      The name of the XML file containing the tagging rules\n"
            "                          (defaults to 'tagging.xml' with '--dir' and\n"
            "                           '--prefix' options or the file installed in\n"
            "                           '" DATADIR "').\n"
            "\n"
            "--loggable                Print progress messages suitable for logging to file.\n"
            "--errorlog[=<name>]       Log parsing errors to 'error.log' or the given name\n"
            "                          (the '--dir' and '--prefix' options are applied).\n"
            "\n"
            "--parse-only              Parse the input OSM files and store the results.\n"
            "--process-only            Process the stored results from previous option.\n"
            "\n"
            "--max-iterations=<number> The number of iterations for finding super-nodes\n"
            "                          (defaults to 5).\n"
            "\n"
            "--prune-none              Disable the prune options below, they are re-enabled\n"
            "                          by adding them to the command line after this option.\n"
            "--prune-isolated=<len>    Remove small disconnected groups of segments\n"
            "                          (defaults to removing groups under 500m).\n"
            "--prune-short=<len>       Remove short segments (defaults to removing segments\n"
            "                          up to a maximum length of 5m).\n"
            "--prune-straight=<len>    Remove nodes in almost straight highways (defaults to\n"
            "                          removing nodes up to 3m offset from a straight line).\n"
            "\n"
            "<filename.osm> ...        The name(s) of the file(s) to process (by default\n"
            "                          data is read from standard input).\n"
            "\n"
            "<transport> defaults to all but can be set to:\n"
            "%s"
            "\n"
            "<highway> can be selected from:\n"
            "%s"
            "\n"
            "<property> can be selected from:\n"
            "%s",
            TransportList(),HighwayList(),PropertyList());

 exit(!detail);
}
