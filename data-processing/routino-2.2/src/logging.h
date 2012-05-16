/***************************************
 Header file for logging function prototypes

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


#ifndef LOGGING_H
#define LOGGING_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>


/* Variables */

extern int option_loggable;


/* Runtime progress logging functions in logging.c */

#ifdef __GNUC__

void printf_first(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void printf_middle(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void printf_last(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

void fprintf_first(FILE *file,const char *format, ...) __attribute__ ((format (printf, 2, 3)));
void fprintf_middle(FILE *file,const char *format, ...) __attribute__ ((format (printf, 2, 3)));
void fprintf_last(FILE *file,const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#else

void printf_first(const char *format, ...);
void printf_middle(const char *format, ...);
void printf_last(const char *format, ...);

void fprintf_first(FILE *file,const char *format, ...);
void fprintf_middle(FILE *file,const char *format, ...);
void fprintf_last(FILE *file,const char *format, ...);

#endif


/* Parsing/processing error logging functions in logging.c */

void open_errorlog(const char *filename,int append);
void close_errorlog(void);

#ifdef __GNUC__

void logerror(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#else

void logerror(const char *format, ...);

#endif


#endif /* LOGGING_H */
