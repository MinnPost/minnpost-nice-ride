/***************************************
 A header file for the profiles.

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


#ifndef PROFILES_H
#define PROFILES_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Data structures */

/*+ A data structure to hold a transport type profile. +*/
typedef struct _Profile
{
 char        *name;                      /*+ The name of the profile. +*/

 Transport    transport;                 /*+ The type of transport. +*/

 transports_t allow;                     /*+ The type of transport expressed as a bitmask. +*/

 score_t      highway[Way_Count];        /*+ A floating point preference for travel on the highway. +*/
 score_t      max_pref;                  /*+ The maximum preference for any highway type. +*/

 speed_t      speed[Way_Count];          /*+ The maximum speed on each type of highway. +*/
 speed_t      max_speed;                 /*+ The maximum speed for any highway type. +*/

 score_t      props_yes[Property_Count]; /*+ A floating point preference for ways with this attribute. +*/
 score_t      props_no [Property_Count]; /*+ A floating point preference for ways without this attribute. +*/

 int          oneway;                    /*+ A flag to indicate if one-way restrictions apply. +*/
 int          turns;                     /*+ A flag to indicate if turn restrictions apply. +*/

 weight_t     weight;                    /*+ The minimum weight of the route. +*/

 height_t     height;                    /*+ The minimum height of vehicles on the route. +*/
 width_t      width;                     /*+ The minimum width of vehicles on the route. +*/
 length_t     length;                    /*+ The minimum length of vehicles on the route. +*/
}
 Profile;


/* Functions in profiles.c */

int ParseXMLProfiles(const char *filename);

Profile *GetProfile(const char *name);

int UpdateProfile(Profile *profile,Ways *ways);

void PrintProfile(const Profile *profile);

void PrintProfilesXML(void);

void PrintProfilesJSON(void);

void PrintProfilesPerl(void);


#endif /* PROFILES_H */
