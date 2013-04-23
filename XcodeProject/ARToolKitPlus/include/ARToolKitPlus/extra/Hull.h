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
* $Id: Hull.h 176 2006-08-04 06:59:22Z daniel $
* @file
* ======================================================================== */
 

#ifndef __ARTOOLKITPLUS_HULL_HEADERFILE__
#define __ARTOOLKITPLUS_HULL_HEADERFILE__


namespace ARToolKitPlus {


enum {
	MAX_HULL_POINTS = 64		// support up to 16 visible markers
};

struct MarkerPoint
{
	typedef int		coord_type;

	coord_type		x,y;
	unsigned short	markerIdx,
					cornerIdx;
};


inline int iabs(int nValue)
{
	return nValue>=0 ? nValue : -nValue;
}


int nearHull_2D(const MarkerPoint* P, int n, int k, MarkerPoint* H);

void findLongestDiameter(const MarkerPoint* nPoints, int nNumPoints, int &nIdx0, int &nIdx1);

void findFurthestAway(const MarkerPoint* nPoints, int nNumPoints, int nIdx0, int nIdx1, int& nIdxFarthest);

void maximizeArea(const MarkerPoint* nPoints, int nNumPoints, int nIdx0, int nIdx1, int nIdx2, int& nIdxMax);

void sortIntegers(int& nIdx0, int& nIdx1, int& nIdx2);

void sortInLastInteger(int& nIdx0, int& nIdx1, int& nIdx2, int &nIdx3);


}  // namespace ARToolKitPlus


#endif //__ARTOOLKITPLUS_HULL_HEADERFILE__
