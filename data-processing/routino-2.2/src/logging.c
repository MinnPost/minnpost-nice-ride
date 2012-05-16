/***************************************
 Functions to handle logging functions.

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


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "logging.h"


/* Global variables */

/*+ The option to print the output in a way that allows logging to a file. +*/
int option_loggable=0;

/* Local functions */

static void vfprintf_first(FILE *file,const char *format,va_list ap);
static void vfprintf_middle(FILE *file,const char *format,va_list ap);
static void vfprintf_last(FILE *file,const char *format,va_list ap);

/* Local variables */

/*+ The length of the string printed out last time. +*/
static int printed_length=0;

/*+ The file handle for the error log file. +*/
static FILE *errorlogfile;


/*++++++++++++++++++++++++++++++++++++++
  Print the first message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_first(const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_first(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the middle message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_middle(const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_middle(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the last message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_last(const char *format, ...)
{
 va_list ap;

 va_start(ap,format);

 vfprintf_last(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the first message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_first(FILE *file,const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_first(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the middle message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_middle(FILE *file,const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_middle(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the last message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_last(FILE *file,const char *format, ...)
{
 va_list ap;

 va_start(ap,format);

 vfprintf_last(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the first message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_first(FILE *file,const char *format,va_list ap)
{
 int retval;

 retval=vfprintf(file,format,ap);
 fflush(file);

 if(retval>0)
    printed_length=retval;
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the middle message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_middle(FILE *file,const char *format,va_list ap)
{
 int retval;

 putchar('\r');
 retval=vfprintf(file,format,ap);
 fflush(file);

 if(retval>0)
   {
    int new_printed_length=retval;

    while(retval++<printed_length)
       putchar(' ');

    printed_length=new_printed_length;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the last message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_last(FILE *file,const char *format,va_list ap)
{
 int retval;

 if(!option_loggable)
    putchar('\r');
 retval=vfprintf(file,format,ap);

 if(retval>0)
    while(retval++<printed_length)
       putchar(' ');

 putchar('\n');
 fflush(file);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the error log file.

  const char *filename The name of the file to create.

  int append The option to append to an existing file.
  ++++++++++++++++++++++++++++++++++++++*/

void open_errorlog(const char *filename,int append)
{
 errorlogfile=fopen(filename,append?"a":"w");

 if(!errorlogfile)
   {
    fprintf(stderr,"Cannot open file '%s' for writing [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Close the error log file.
  ++++++++++++++++++++++++++++++++++++++*/

void close_errorlog(void)
{
 if(errorlogfile)
    fclose(errorlogfile);
}


/*++++++++++++++++++++++++++++++++++++++
  Log a message to the error log file.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void logerror(const char *format, ...)
{
 va_list ap;

 if(!errorlogfile)
    return;

 va_start(ap,format);

 vfprintf(errorlogfile,format,ap);

 va_end(ap);
}
