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
* $Id: Hull.cpp 176 2006-08-04 06:59:22Z daniel $
* @file
* ======================================================================== */


#include <ARToolKitPlus/extra/Hull.h>
#include <assert.h>


namespace ARToolKitPlus {


//
// Copyright notice for the function nearHull_2D:
//
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
//

// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
//
inline MarkerPoint::coord_type
isLeft(const MarkerPoint& P0, const MarkerPoint& P1, const MarkerPoint& P2)
{
	return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
}
//===================================================================


#define NONE  (-1)

typedef struct range_bin Bin;
struct range_bin {
	int    min;    // index of min point P[] in bin (>=0 or NONE)
	int    max;    // index of max point P[] in bin (>=0 or NONE)
};


// nearHull_2D(): the BFP fast approximate 2D convex hull algorithm
//     Input:  P[] = an (unsorted) array of 2D points
//              n = the number of points in P[]
//              k = the approximation accuracy (large k = more accurate)
//     Output: H[] = an array of the convex hull vertices (max is n)
//     Return: the number of points in H[]
//
int
nearHull_2D(const MarkerPoint* P, int n, int k, MarkerPoint* H)
{
	int						minmin=0,  minmax=0;
	int						maxmin=0,  maxmax=0;
	MarkerPoint::coord_type	xmin = P[0].x,  xmax = P[0].x;
	const MarkerPoint*		cP;                 // the current point being considered
	int						bot=0, top=(-1);  // indices for bottom and top of the stack

	// Get the points with (1) min-max x-coord, and (2) min-max y-coord
	for (int i=1; i<n; i++) {
		cP = &P[i];
		if (cP->x <= xmin) {
			if (cP->x < xmin) {        // new xmin
				xmin = cP->x;
				minmin = minmax = i;
			}
			else {                      // another xmin
				if (cP->y < P[minmin].y)
					minmin = i;
				else if (cP->y > P[minmax].y)
					minmax = i;
			}
		}
		if (cP->x >= xmax) {
			if (cP->x > xmax) {        // new xmax
				xmax = cP->x;
				maxmin = maxmax = i;
			}
			else {                      // another xmax
				if (cP->y < P[maxmin].y)
					maxmin = i;
				else if (cP->y > P[maxmax].y)
					maxmax = i;
			}
		}
	}
	if (xmin == xmax) {      // degenerate case: all x-coords == xmin
		H[++top] = P[minmin];           // a point, or
		if (minmax != minmin)           // a nontrivial segment
			H[++top] = P[minmax];
		return top+1;                   // one or two points
	}

	// Next, get the max and min points in the k range bins
	Bin*   B = new Bin[k+2];   // first allocate the bins
	B[0].min = minmin;         B[0].max = minmax;        // set bin 0
	B[k+1].min = maxmin;       B[k+1].max = maxmax;      // set bin k+1
	for (int b=1; b<=k; b++) { // initially nothing is in the other bins
		B[b].min = B[b].max = NONE;
	}
	for (int b, i=0; i<n; i++) {
		cP = &P[i];
		if (cP->x == xmin || cP->x == xmax) // already have bins 0 and k+1 
			continue;
		// check if a lower or upper point
		if (isLeft( P[minmin], P[maxmin], *cP) < 0) {  // below lower line
			b = (int)( k * (cP->x - xmin) / (xmax - xmin) ) + 1;  // bin #
			if (B[b].min == NONE)       // no min point in this range
				B[b].min = i;           // first min
			else if (cP->y < P[B[b].min].y)
				B[b].min = i;           // new min
			continue;
		}
		if (isLeft( P[minmax], P[maxmax], *cP) > 0) {  // above upper line
			b = (int)( k * (cP->x - xmin) / (xmax - xmin) ) + 1;  // bin #
			if (B[b].max == NONE)       // no max point in this range
				B[b].max = i;           // first max
			else if (cP->y > P[B[b].max].y)
				B[b].max = i;           // new max
			continue;
		}
	}

	// Now, use the chain algorithm to get the lower and upper hulls
	// the output array H[] will be used as the stack
	// First, compute the lower hull on the stack H
	for (int i=0; i <= k+1; ++i)
	{
		if (B[i].min == NONE)  // no min point in this range
			continue;
		cP = &P[ B[i].min ];   // select the current min point

		while (top > 0)        // there are at least 2 points on the stack
		{
			// test if current point is left of the line at the stack top
			if (isLeft( H[top-1], H[top], *cP) > 0)
				break;         // cP is a new hull vertex
			else
				top--;         // pop top point off stack
		}
		H[++top] = *cP;        // push current point onto stack
	}

	// Next, compute the upper hull on the stack H above the bottom hull
	if (maxmax != maxmin)      // if distinct xmax points
		H[++top] = P[maxmax];  // push maxmax point onto stack
	bot = top;                 // the bottom point of the upper hull stack
	for (int i=k; i >= 0; --i)
	{
		if (B[i].max == NONE)  // no max point in this range
			continue;
		cP = &P[ B[i].max ];   // select the current max point

		while (top > bot)      // at least 2 points on the upper stack
		{
			// test if current point is left of the line at the stack top
			if (isLeft( H[top-1], H[top], *cP) > 0)
				break;         // current point is a new hull vertex
			else
				top--;         // pop top point off stack
		}
		H[++top] = *cP;        // push current point onto stack
	}
	if (minmax != minmin)
		H[++top] = P[minmin];  // push joining endpoint onto stack

	delete B;                  // free bins before returning
	return top+1;              // # of points on the stack
}


inline int distanceSquare(const MarkerPoint& nPoint1, const MarkerPoint& nPoint2)
{
	int dx=nPoint1.x-nPoint2.x;
	int dy=nPoint1.y-nPoint2.y;

	return dx*dx+dy*dy;
}


void
swap(int& nIdx0, int& nIdx1)
{
	int tmp = nIdx0;
	nIdx0 = nIdx1;
	nIdx1 = tmp;
}

void
sortIntegers(int& nIdx0, int& nIdx1, int& nIdx2)
{
	if(nIdx0>nIdx1)
		swap(nIdx0,nIdx1);
	if(nIdx1>nIdx2)
		swap(nIdx1,nIdx2);
	if(nIdx0>nIdx1)
		swap(nIdx0,nIdx1);
}


void
sortInLastInteger(int& nIdx0, int& nIdx1, int& nIdx2, int &nIdx3)
{
	int tmp;

	if(nIdx3<nIdx0)
	{
		tmp = nIdx3;
		nIdx3 = nIdx2;
		nIdx2 = nIdx1;
		nIdx1 = nIdx0;
		nIdx0 = tmp;
	}
	else
	if(nIdx3<nIdx1)
	{
		tmp = nIdx3;
		nIdx3 = nIdx2;
		nIdx2 = nIdx1;
		nIdx1 = tmp;
	}
	else
	if(nIdx3<nIdx2)
	{
		tmp = nIdx3;
		nIdx3 = nIdx2;
		nIdx2 = tmp;
	}
	else
	{
		assert(nIdx3>nIdx2);
	}
}


int
calcArea(const MarkerPoint& nPoint0, const MarkerPoint& nPoint1, const MarkerPoint& nPoint2, const MarkerPoint& nPoint3)
{
	// this will return twice the area, but since we only need it for comparison, we don't care...
	//
	return    (nPoint0.x*nPoint1.y + nPoint1.x*nPoint2.y + nPoint2.x*nPoint3.y + nPoint3.x*nPoint0.y)
			- (nPoint0.y*nPoint1.x + nPoint1.y*nPoint2.x + nPoint2.y*nPoint3.x + nPoint3.y*nPoint0.x);
}


void findLongestDiameter(const MarkerPoint* nPoints, int nNumPoints, int &nIdx0, int &nIdx1)
{
	int half = (nNumPoints+1)/2;
	int maxDist=-1;

	if(nNumPoints>5)
	{
		for(int i=0; i<half; i++)
		{
			for(int j=-1; j<=1; j++)
			{
				int idx1 = i+half+j;
				if(idx1>=nNumPoints)
					idx1 -= nNumPoints;

				int dist = distanceSquare(nPoints[i], nPoints[idx1]);
				if(dist>maxDist)
				{
					maxDist = dist;
					nIdx0 = i;
					nIdx1 = idx1;
				}
			}
		}
	}
	else
	{
		for(int i=0; i<half; i++)
		{
			int idx1 = i+half;
			if(idx1>=nNumPoints)
				idx1 -= nNumPoints;

			int dist = distanceSquare(nPoints[i], nPoints[idx1]);
			if(dist>maxDist)
			{
				maxDist = dist;
				nIdx0 = i;
				nIdx1 = idx1;
			}
		}

	}
}


void
findFurthestAway(const MarkerPoint* nPoints, int nNumPoints, int nIdx0, int nIdx1, int& nIdxFarthest)
{
	int maxDist=0;

	for(int i=0; i<nNumPoints; i++)
	{
		if(i!=nIdx0 && i!=nIdx1)
		{
			int dist = isLeft(nPoints[nIdx0], nPoints[nIdx1], nPoints[i]);
			dist = iabs(dist);
			if(dist>maxDist)
			{
				maxDist = dist;
				nIdxFarthest = i;
			}
		}
	}
}


void
maximizeArea(const MarkerPoint* nPoints, int nNumPoints, int nIdx0, int nIdx1, int nIdx2, int& nIdxMax)
{
	int maxArea = 0;

	for(int i=0; i<nIdx0; i++)
	{
		int area = calcArea(nPoints[i], nPoints[nIdx0], nPoints[nIdx1], nPoints[nIdx2]);
		if(area>maxArea)
		{
			maxArea = area;
			nIdxMax = i;
		}
	}

	for(int i=nIdx0+1; i<nIdx1; i++)
	{
		int area = calcArea(nPoints[nIdx0], nPoints[i], nPoints[nIdx1], nPoints[nIdx2]);
		if(area>maxArea)
		{
			maxArea = area;
			nIdxMax = i;
		}
	}

	for(int i=nIdx1+1; i<nIdx2; i++)
	{
		int area = calcArea(nPoints[nIdx0], nPoints[nIdx1], nPoints[i], nPoints[nIdx2]);
		if(area>maxArea)
		{
			maxArea = area;
			nIdxMax = i;
		}
	}

	for(int i=nIdx2+1; i<nNumPoints; i++)
	{
		int area = calcArea(nPoints[nIdx0], nPoints[nIdx1], nPoints[nIdx2], nPoints[i]);
		if(area>maxArea)
		{
			maxArea = area;
			nIdxMax = i;
		}
	}
}


}  // namespace ARToolKitPlus
