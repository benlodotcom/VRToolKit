/* ========================================================================
* PROJECT: ARToolKitPlus
* ========================================================================
* This work is based on the original ARToolKit developed by
*   Hirokazu Kato
*   Mark Billinghurst
*   HITLab, University of Washington, Seattle
* http://www.hitl.washington.edu/artoolkit/
*
* Copyright of the derived and new portions of this work
*     (C) 2006 Graz University of Technology
*
* This framework is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this framework; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* For further information please contact 
*   Dieter Schmalstieg
*   <schmalstieg@icg.tu-graz.ac.at>
*   Graz University of Technology, 
*   Institut for Computer Graphics and Vision,
*   Inffeldgasse 16a, 8010 Graz, Austria.
* ========================================================================
** @author   Daniel Wagner
*
* $Id: arMulti.h 162 2006-04-19 21:28:10Z grabner $
* @file
* ======================================================================== */


#ifndef __ARTOOLKITMULTI_HEADERFILE__
#define __ARTOOLKITMULTI_HEADERFILE__


#include <stdio.h>
#include <ARToolKitPlus/config.h>
#include <ARToolKitPlus/param.h>
#include <ARToolKitPlus/ar.h>


namespace ARToolKitPlus {


typedef struct {
    int     patt_id;
    ARFloat  width;
    ARFloat  center[2];
    ARFloat  trans[3][4];
    ARFloat  itrans[3][4];
    ARFloat  pos3d[4][3];
    int     visible;
/*---*/
    int     visibleR;
} ARMultiEachMarkerInfoT;

typedef struct {
    ARMultiEachMarkerInfoT  *marker;
    int                     marker_num;
    ARFloat                  trans[3][4];
    int                     prevF;
/*---*/
    ARFloat                  transR[3][4];
} ARMultiMarkerInfoT;


} // namespace ARToolKitPlus


#endif // __ARTOOLKITMULTI_HEADERFILE__
