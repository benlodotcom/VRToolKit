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
 * $Id: arMultiGetTransMatHull.cxx 178 2006-08-09 17:51:13Z daniel $
 * @file
 * ======================================================================== */


#include <ARToolKitPlus/extra/Hull.h>
#include <ARToolKitPlus/arGetInitRot2Sub.h>


namespace ARToolKitPlus {


AR_TEMPL_FUNC ARFloat
AR_TEMPL_TRACKER::arMultiGetTransMatHull(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config)
{
	//return arMultiGetTransMat(marker_info, marker_num, config);

	int numInPoints=0;
	std::vector<int> trackedMarkers;
	int trackedCenterX=-1,trackedCenterY=-1;

	//const int indices[4] = {idx0,idx1,idx2,idx3};
	//rpp_vec ppos2d[4];
	//rpp_vec ppos3d[4];

	const int maxHullPoints = 16;
	int indices[maxHullPoints];
	rpp_vec ppos2d[maxHullPoints];
	rpp_vec ppos3d[maxHullPoints];


	// create an array of 2D points and keep references
	// to the source points
	//
	for(int i=0; i<marker_num; i++)
	{
		int mId = marker_info[i].id;
		int configIdx = -1;

		for(int j=0; j<config->marker_num; j++)
			if(config->marker[j].patt_id==mId)
			{
				configIdx = j;
				break;
			}

		if(configIdx==-1)
			continue;

		trackedMarkers.push_back(i);

		for(int c=0; c<4; c++)
		{
			int dir = marker_info[i].dir;
			int cornerIdx = (c+4-dir)%4;
			hullInPoints[numInPoints].x = (MarkerPoint::coord_type)marker_info[i].vertex[cornerIdx][0];
			hullInPoints[numInPoints].y = (MarkerPoint::coord_type)marker_info[i].vertex[cornerIdx][1];
			hullInPoints[numInPoints].cornerIdx = c;
			hullInPoints[numInPoints].markerIdx = configIdx;
			numInPoints++;
		}

		if(numInPoints>=MAX_HULL_POINTS)
			break;
	}


	// next get the convex hull of all points
	// (decrease amount by one to ignore last point, which is identical to first point)
	//
	int numHullPoints = nearHull_2D(hullInPoints, numInPoints, numInPoints, hullOutPoints)-1;
	int idx0,idx1,idx2,idx3;


	if(hullTrackingMode==HULL_FOUR)
	{
		// find those points with furthest distance and that lie on
		// opposite parts of the hull. this fixes the first two points of our quad.
		//
		findLongestDiameter(hullOutPoints, numHullPoints, idx0,idx1);
		assert(iabs(idx0-idx1)>0);

		// find the point that is furthest away of the line
		// of our first two points. this fixes the third point of the quad
		findFurthestAway(hullOutPoints, numHullPoints, idx0,idx1, idx2);
		sortIntegers(idx0,idx1, idx2);

		// of all other points find the one that results in
		// a quad with the largest area.
		maximizeArea(hullOutPoints, numHullPoints, idx0,idx1,idx2,idx3);

		// now that we have all four points we must sort them...
		//
		sortInLastInteger(idx0,idx1,idx2, idx3);

		numHullPoints = 4;
		indices[0] = idx0;
		indices[1] = idx1;
		indices[2] = idx2;
		indices[3] = idx3;
	}
	else
	{
		if(numHullPoints>maxHullPoints)
			numHullPoints = maxHullPoints;

		for(int i=0; i<numHullPoints; i++)
			indices[i] = i;
	}

	assert(numHullPoints<=maxHullPoints);

	// create arrays of vertices for the 2D and 3D positions
	// 
	//const int indices[4] = {idx0,idx1,idx2,idx3};
	//rpp_vec ppos2d[4];
	//rpp_vec ppos3d[4];

	for(int i=0; i<numHullPoints; i++)
	{
		//int idx = indices[(i+1)%4];
		int idx = indices[i];
		const MarkerPoint& pt = hullOutPoints[idx];

		trackedCorners.push_back(CornerPoint(pt.x,pt.y));

		trackedCenterX += pt.x;
		trackedCenterY += pt.y;

		ppos2d[i][0] = pt.x;
		ppos2d[i][1] = pt.y;
		ppos2d[i][2] = 1.0f;

		assert(pt.markerIdx < config->marker_num);
		const ARMultiEachMarkerInfoT& markerInfo = config->marker[pt.markerIdx];
		int cornerIdx = pt.cornerIdx;

		ppos3d[i][0] = markerInfo.pos3d[cornerIdx][0];
		ppos3d[i][1] = markerInfo.pos3d[cornerIdx][1];
		ppos3d[i][2] = 0;
	}

	trackedCenterX /= 4;
	trackedCenterY /= 4;


	// prepare structures and data we need for input and output
	// parameters of the rpp functions
	const rpp_float cc[2] = {arCamera->mat[0][2],arCamera->mat[1][2]};
	const rpp_float fc[2] = {arCamera->mat[0][0],arCamera->mat[1][1]};
	rpp_float err = 1e+20;
	rpp_mat R, R_init;
	rpp_vec t;

	if(poseEstimator==POSE_ESTIMATOR_RPP)
	{
		robustPlanarPose(err,R,t,cc,fc,ppos3d,ppos2d,numHullPoints,R_init, true,0,0,0);
		if(err>1e+10)
			return(-1); // an actual error has occurred in robustPlanarPose()

		for(int k=0; k<3; k++)
		{
			config->trans[k][3] = (ARFloat)t[k];
			for(int j=0; j<3; j++)
				config->trans[k][j] = (ARFloat)R[k][j];
		}
	}
	else
	{
		ARFloat rot[3][3];

		int minIdx=-1, minDist=0x7fffffff;
		for(size_t i=0; i<trackedMarkers.size(); i++)
		{
			assert(trackedMarkers[i]>=0 && trackedMarkers[i]<marker_num);
			int idx = trackedMarkers[i];
			const ARMarkerInfo& mInfo = marker_info[idx];
			int dx = trackedCenterX-(int)mInfo.pos[0], dy = trackedCenterY-(int)mInfo.pos[1];
			int d = dx*dx + dy*dy;
			if(d<minDist)
			{
				minDist = d;
				minIdx = (int)idx;
			}
		}

		assert(minIdx>=0);
		//trackedCorners.push_back(CornerPoint((int)marker_info[minIdx].pos[0],(int)marker_info[minIdx].pos[1]));

		if(arGetInitRot(marker_info+minIdx, arCamera->mat, rot )<0)
			return 99.0f;


		// finally use the normal pose estimator to get the pose
		//
		ARFloat tmp_pos2d[maxHullPoints][2], tmp_pos3d[maxHullPoints][2];

		for(int i=0; i<numHullPoints; i++)
		{
			tmp_pos2d[i][0] = (ARFloat)ppos2d[i][0];
			tmp_pos2d[i][1] = (ARFloat)ppos2d[i][1];
			tmp_pos3d[i][0] = (ARFloat)ppos3d[i][0];
			tmp_pos3d[i][1] = (ARFloat)ppos3d[i][1];
		}

		for(int i=0; i<AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ )
		{
			err = arGetTransMat3(rot, tmp_pos2d, tmp_pos3d, numHullPoints, config->trans, arCamera);
			if(err<AR_GET_TRANS_MAT_MAX_FIT_ERROR)
				break;
		}
	}

    return (ARFloat)err;
}



}  // namespace ARToolKitPlus
