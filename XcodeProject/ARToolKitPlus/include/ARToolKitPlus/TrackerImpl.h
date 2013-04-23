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
* $Id: TrackerImpl.h 172 2006-07-25 14:05:47Z daniel $
* @file
* ======================================================================== */


#ifndef __ARTOOLKIT_TRACKERIMPL_HEADERFILE__
#define __ARTOOLKIT_TRACKERIMPL_HEADERFILE__


#include <ARToolKitPlus/ar.h>
#include <ARToolKitPlus/arMulti.h>
#include <ARToolKitPlus/matrix.h>
#include <ARToolKitPlus/Tracker.h>
#include <ARToolKitPlus/MemoryManager.h>
#include <ARToolKitPlus/Camera.h>
#include <ARToolKitPlus/CameraFactory.h>
#include <ARToolKitPlus/extra/BCH.h>
#include <ARToolKitPlus/extra/Hull.h>


#define AR_TEMPL_FUNC template <int __PATTERN_SIZE_X, int __PATTERN_SIZE_Y, int __PATTERN_SAMPLE_NUM, int __MAX_LOAD_PATTERNS, int __MAX_IMAGE_PATTERNS>
#define AR_TEMPL_TRACKER TrackerImpl<__PATTERN_SIZE_X, __PATTERN_SIZE_Y, __PATTERN_SAMPLE_NUM, __MAX_LOAD_PATTERNS, __MAX_IMAGE_PATTERNS>


namespace ARToolKitPlus {


// Compile time information...
//
static bool usesSinglePrecision();				/// Returns whether single or double precision is used
//const char* getDescriptionString();		/// Returns a short description string about compile time settings

#ifndef _ARTKP_NO_MEMORYMANAGER_
extern MemoryManager* memManager;
#endif //_ARTKP_NO_MEMORYMANAGER_

void artkp_Free(void* nRawMemory);

template<class T> T* artkp_Alloc(size_t size)
{
#ifndef _ARTKP_NO_MEMORYMANAGER_
	if(memManager)
		return (T*)memManager->getMemory(size*sizeof(T));
	else
#endif //_ARTKP_NO_MEMORYMANAGER_
		return (T*)::malloc(size*sizeof(T));
}


/// TrackerImpl implements the Tracker interface
template <int __PATTERN_SIZE_X, int __PATTERN_SIZE_Y, int __PATTERN_SAMPLE_NUM, int __MAX_LOAD_PATTERNS, int __MAX_IMAGE_PATTERNS>
class TrackerImpl : public Tracker
{
public:
	enum {
		PATTERN_WIDTH = __PATTERN_SIZE_X,
		PATTERN_HEIGHT = __PATTERN_SIZE_Y,
		PATTERN_SAMPLE_NUM = __PATTERN_SAMPLE_NUM,

		MAX_LOAD_PATTERNS = __MAX_LOAD_PATTERNS,
		MAX_IMAGE_PATTERNS = __MAX_IMAGE_PATTERNS,
		WORK_SIZE = 1024*MAX_IMAGE_PATTERNS,

#ifdef SMALL_LUM8_TABLE
		LUM_TABLE_SIZE = (0xffff >> 6) + 1,
#else
		LUM_TABLE_SIZE = 0xffff + 1,
#endif
	};


	TrackerImpl();
	virtual ~TrackerImpl();

	/// does final clean up (memory deallocation)
	virtual void cleanup();


	/// Sets the pixel format of the camera image
	/**
	 *  Default format is RGB888 (PIXEL_FORMAT_RGB)
	 */
	virtual bool setPixelFormat(PIXEL_FORMAT nFormat);

	/// Loads a camera calibration file and stores data internally
	/**
	 *  To prevent memory leaks, this method internally deletes an existing camera.
	 *  If you want to use more than one camera, retrieve the existing camera using getCamera()
	 *  and call setCamera(NULL); before loading another camera file.
	 *  On destruction, ARToolKitPlus will only destroy the currently set camera. All other
	 *  cameras have to be destroyed manually.
	 */
	virtual bool loadCameraFile(const char* nCamParamFile, ARFloat nNearClip, ARFloat nFarClip);

	virtual void setLoadUndistLUT(bool nSet)  {  loadCachedUndist = nSet;  }

	/// sets an instance which implements the ARToolKit::Logger interface
	virtual void setLogger(ARToolKitPlus::Logger* nLogger)  {  logger = nLogger;  }

	/// marker detection using tracking history
	virtual int arDetectMarker(uint8_t *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num);

	/// marker detection without using tracking history
	virtual int arDetectMarkerLite(uint8_t *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num);

	/// calculates the transformation matrix between camera and the given multi-marker config
	virtual ARFloat arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

    virtual ARFloat arMultiGetTransMatHull(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

	/// calculates the transformation matrix between camera and the given marker
	virtual ARFloat arGetTransMat(ARMarkerInfo *marker_info, ARFloat center[2], ARFloat width, ARFloat conv[3][4]);

	virtual ARFloat arGetTransMatCont(ARMarkerInfo *marker_info, ARFloat prev_conv[3][4], ARFloat center[2], ARFloat width, ARFloat conv[3][4]);

	// RPP integration -- [t.pintaric]
	virtual ARFloat rppMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);
	virtual ARFloat rppGetTransMat(ARMarkerInfo *marker_info, ARFloat center[2], ARFloat width, ARFloat conv[3][4]);

	/// loads a pattern from a file
	virtual int arLoadPatt(char *filename);

	/// frees a pattern from memory
	virtual int arFreePatt(int patno);

	virtual int arMultiFreeConfig( ARMultiMarkerInfoT *config );

	virtual ARMultiMarkerInfoT *arMultiReadConfigFile(const char *filename);

	virtual void activateBinaryMarker(int nThreshold)  {  binaryMarkerThreshold = nThreshold;  }

	/// Activate the usage of id-based markers rather than template based markers
	/**
	 *  id-based markers directly encode the marker id in the image.
	 *  see arBitFieldPattern.h for more information
	 */
	virtual void setMarkerMode(MARKER_MODE nMarkerMode);


	/// activates the complensation of brightness falloff in the corners of the camera image
	/**
	 *  some cameras have a falloff in brightness at the border of the image, which creates
	 *  problems with thresholding the image. use this function to set a (linear) adapted
	 *  threshold value. the threshold value will stay exactly the same at the center but
	 *  will deviate near to the border. all values specify a difference, not absolute values!
	 *  nCorners define the falloff a all four corners. nLeftRight defines the falloff
	 *  at the half y-position at the left and right side of the image. nTopBottom defines the falloff
	 *  at the half x-position at the top and bottom side of the image.
	 *  all values between these 9 points (center, 4 corners, left, right, top, bottom) will
	 *  be interpolated.
	 */
	virtual void activateVignettingCompensation(bool nEnable, int nCorners=0, int nLeftRight=0, int nTopBottom=0);


	/// Calculates the camera matrix from an ARToolKit camera file.
	/**
	 * This method retrieves the OpenGL projection matrix that is stored
	 * in an ARToolKit camera calibration file.
	 * Returns true if loading of the camera file succeeded.
	 */
	static bool calcCameraMatrix(const char* nCamParamFile, int nWidth, int nHeight,
								 ARFloat nNear, ARFloat nFar, ARFloat *nMatrix);

	
	/// Changes the resolution of the camera after the camerafile was already loaded
	virtual void changeCameraSize(int nWidth, int nHeight);


	/// Changes the undistortion mode
	/**
	 * Default value is UNDIST_STD which means that
	 * artoolkit's standard undistortion method is used.
	 */
	virtual void setUndistortionMode(UNDIST_MODE nMode);

	/// Changes the Pose Estimation Algorithm
	/**
	* POSE_ESTIMATOR_ORIGINAL (default): arGetTransMat()
	* POSE_ESTIMATOR_CONT: original pose estimator with "Cont"
	* POSE_ESTIMATOR_RPP: "Robust Pose Estimation from a Planar Target"
	*/
	virtual bool setPoseEstimator(POSE_ESTIMATOR nMethod);


	/// If true the alternative hull-algorithm will be used for multi-marker tracking
	/**
	 *  Starting with version 2.2 ARToolKitPlus has a new mode for tracking multi-markers:
	 *  Instead of using all points (as done by RPP multi-marker tracking)
	 *  or tracking all markers independently and combine lateron
	 *  (as done in ARToolKit's standard multi-marker pose estimator), ARToolKitPlus can now
	 *  use only 4 'good' points of the convex hull to do the pose estimation.
	 *  If the pose estimator is set to RPP then RPP will be used to track those 4 points.
	 *  Otherwise, ARToolKit's standard single-marker pose estimator will be used to
	 *  track the pose of these 4 points.
	 */
	virtual void setHullMode(HULL_TRACKING_MODE nMode)  {  hullTrackingMode = nMode;  }

	/// Sets a new relative border width. ARToolKit's default value is 0.25
	/**
	 * Take caution that the markers need of course really have thiner borders.
	 * Values other than 0.25 have not been tested for regular pattern-based matching,
	 * but only for id-encoded markers. It might be that the pattern creation process
	 * needs to be updated too.
	 */
	virtual void setBorderWidth(ARFloat nFraction)  {  relBorderWidth = nFraction;  }


	/// Sets the threshold value that is used for black/white conversion
	virtual void setThreshold(int nValue)  {  thresh = nValue;  }


	/// Returns the current threshold value.
	virtual int getThreshold() const  {  return thresh;  }


	/// Turns automatic threshold calculation on/off
	virtual void activateAutoThreshold(bool nEnable)  {  autoThreshold.enable = nEnable;  }


	/// Returns true if automatic threshold detection is enabled
	virtual bool isAutoThresholdActivated() const  {  return autoThreshold.enable;  }

	/// Sets the number of times the threshold is randomized in case no marker was visible (Minimum: 1, Default: 2)
	/**
	 *  Autothreshold requires a visible marker to estime the optimal thresholding value. If
	 *  no marker is visible ARToolKitPlus randomizes the thresholding value until a marker is
	 *  found. This function sets the number of times ARToolKitPlus will randomize the threshold
	 *  value and research for a marker per calc() invokation until it gives up.
	 *  A value of 2 means that ARToolKitPlus will analyze the image a second time with an other treshold value
	 *  if it does not find a marker the first time. Each unsuccessful try uses less processing power
	 *  than a single full successful position estimation.
	 */
	virtual void setNumAutoThresholdRetries(int nNumRetries)  {  autoThreshold.numRandomRetries = nNumRetries>=1 ? nNumRetries : 1;  }


	/// Sets an image processing mode (half or full resolution)
	/**
	 *  Half resolution is faster but less accurate. When using
	 *  full resolution smaller markers will be detected at a
	 *  higher accuracy (or even detected at all).
	 */
	virtual void setImageProcessingMode(IMAGE_PROC_MODE nMode)  {  arImageProcMode = (nMode==IMAGE_HALF_RES ? AR_IMAGE_PROC_IN_HALF : AR_IMAGE_PROC_IN_FULL);  }


	/// Returns an opengl-style modelview transformation matrix
	virtual const ARFloat* getModelViewMatrix() const  {  return gl_para;  }


	/// Returns an opengl-style projection transformation matrix
	virtual const ARFloat* getProjectionMatrix() const  {  return gl_cpara;  }


	/// Returns a short description with compiled-in settings
	virtual const char* getDescription();


	/// Returns the compiled-in pixel format
	virtual PIXEL_FORMAT getPixelFormat() const  {  return static_cast<PIXEL_FORMAT>(pixelFormat);  }


	/// Returns the numbber of bits per pixel for the compiled-in pixel format
	virtual int getBitsPerPixel() const  {  return pixelSize*8;  }


	/// Returns the maximum number of patterns that can be loaded
	virtual int getNumLoadablePatterns() const  {  return MAX_LOAD_PATTERNS;  }


	/// Returns the current camera
	virtual Camera* getCamera()  {  return arCamera;  }


	/// Sets a new camera without specifying new near and far clip values
	virtual void setCamera(Camera* nCamera);


	/// Sets a new camera including specifying new near and far clip values
	virtual void setCamera(Camera* nCamera, ARFloat nNearClip, ARFloat nFarClip);


	virtual ARFloat calcOpenGLMatrixFromMarker(ARMarkerInfo* nMarkerInfo, ARFloat nPatternCenter[2], ARFloat nPatternSize, ARFloat *nOpenGLMatrix);


	virtual ARFloat executeSingleMarkerPoseEstimator(ARMarkerInfo *marker_info, ARFloat center[2], ARFloat width, ARFloat conv[3][4]);


	virtual ARFloat executeMultiMarkerPoseEstimator(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);
	
	virtual const CornerPoints& getTrackedCorners() const  {  return trackedCorners;  }

protected:
	bool checkPixelFormat();

	void checkImageBuffer();

	//static int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam );

	//static int arParamSave( char *filename, int num, ARParam *param, ...);
	//static int arParamLoad( char *filename, int num, ARParam *param, ...);

	// converts an ARToolKit transformation matrix for usage with OpenGL
	void convertTransformationMatrixToOpenGLStyle(ARFloat para[3][4], ARFloat gl_para[16]);

	// converts an ARToolKit projection matrix for usage with OpenGL
	static bool convertProjectionMatrixToOpenGLStyle(ARParam *param, ARFloat gnear, ARFloat gfar, ARFloat m[16]);
	static bool convertProjectionMatrixToOpenGLStyle2(ARFloat cparam[3][4], int width, int height, ARFloat gnear, ARFloat gfar, ARFloat m[16]);


	ARMarkerInfo2* arDetectMarker2(int16_t *limage, int label_num, int *label_ref,
								   int *warea, ARFloat *wpos, int *wclip,
								   int area_max, int area_min, ARFloat factor, int *marker_num);

	int arGetContour(int16_t *limage, int *label_ref, int label, int clip[4], ARMarkerInfo2 *marker_infoTWO);

	int check_square(int area, ARMarkerInfo2 *marker_infoTWO, ARFloat factor);

	int arGetCode(uint8_t *image, int *x_coord, int *y_coord, int *vertex,
				  int *code, int *dir, ARFloat *cf, int thresh);

	int arGetPatt(uint8_t *image, int *x_coord, int *y_coord, int *vertex,
				  uint8_t ext_pat[PATTERN_HEIGHT][PATTERN_WIDTH][3]);

	int pattern_match( uint8_t *data, int *code, int *dir, ARFloat *cf);

	int downsamplePattern(uint8_t* data, unsigned char* imgPtr);

	int bitfield_check_simple(uint8_t *data, int *code, int *dir, ARFloat *cf, int thresh);

	int bitfield_check_BCH(uint8_t *data, int *code, int *dir, ARFloat *cf, int thresh);

	void gen_evec(void);

	ARMarkerInfo* arGetMarkerInfo(uint8_t *image, ARMarkerInfo2 *marker_info2, int *marker_num, int thresh);

	ARFloat arGetTransMat2(ARFloat rot[3][3], ARFloat ppos2d[][2], ARFloat ppos3d[][2], int num, ARFloat conv[3][4]);


	ARFloat arGetTransMat4(ARFloat rot[3][3], ARFloat ppos2d[][2], ARFloat ppos3d[][3], int num, ARFloat conv[3][4]);

	ARFloat arGetTransMat5(ARFloat rot[3][3], ARFloat ppos2d[][2],
						   ARFloat ppos3d[][3], int num, ARFloat conv[3][4],
						   Camera *pCam);
						   //ARFloat *dist_factor, ARFloat cpara[3][4]);

	ARFloat arGetTransMatSub(ARFloat rot[3][3], ARFloat ppos2d[][2],
							 ARFloat pos3d[][3], int num, ARFloat conv[3][4],
							 Camera *pCam);
							 //ARFloat *dist_factor, ARFloat cpara[3][4] );

	ARFloat arModifyMatrix(ARFloat rot[3][3], ARFloat trans[3], ARFloat cpara[3][4],
								  ARFloat vertex[][3], ARFloat pos2d[][2], int num);

	ARFloat arModifyMatrix2(ARFloat rot[3][3], ARFloat trans[3], ARFloat cpara[3][4],
								   ARFloat vertex[][3], ARFloat pos2d[][2], int num);

	int arGetAngle(ARFloat rot[3][3], ARFloat *wa, ARFloat *wb, ARFloat *wc);

	int arGetRot(ARFloat a, ARFloat b, ARFloat c, ARFloat rot[3][3]);

	int arGetNewMatrix(ARFloat a, ARFloat b, ARFloat c,
							  ARFloat trans[3], ARFloat trans2[3][4],
							  ARFloat cpara[3][4], ARFloat ret[3][4]);

	int arGetInitRot(ARMarkerInfo *marker_info, ARFloat cpara[3][4], ARFloat rot[3][3]);

    int arGetInitRot2(ARMarkerInfo *marker_info, ARFloat cpara[3][4], ARFloat rot[3][3], ARFloat center[2], ARFloat width);

	ARFloat arGetTransMatCont2(ARMarkerInfo *marker_info, ARFloat center[2], ARFloat width, ARFloat conv[3][4]);

	ARFloat arGetTransMatContSub(ARMarkerInfo *marker_info, ARFloat prev_conv[3][4], ARFloat center[2], ARFloat width, ARFloat conv[3][4]);



	int16_t* arLabeling(uint8_t *image, int thresh,int *label_num, int **area,
						ARFloat **pos, int **clip, int **label_ref );


	int16_t* arLabeling_ABGR(uint8_t *image, int thresh,int *label_num, int **area, ARFloat **pos, int **clip, int **label_ref);
	int16_t* arLabeling_BGR(uint8_t *image, int thresh,int *label_num, int **area, ARFloat **pos, int **clip, int **label_ref);
	int16_t* arLabeling_RGB(uint8_t *image, int thresh,int *label_num, int **area, ARFloat **pos, int **clip, int **label_ref);
	int16_t* arLabeling_RGB565(uint8_t *image, int thresh,int *label_num, int **area, ARFloat **pos, int **clip, int **label_ref);
	int16_t* arLabeling_LUM(uint8_t *image, int thresh,int *label_num, int **area, ARFloat **pos, int **clip, int **label_ref);

	//int16_t* labeling2(uint8_t *image, int thresh,int *label_num, int **area,
	//				   ARFloat **pos, int **clip, int **label_ref, int LorR );

	//int16_t* labeling3(uint8_t *image, int thresh, int *label_num, int **area,
	//				   ARFloat **pos, int **clip, int **label_ref, int LorR );


	int arActivatePatt(int patno);

	int arDeactivatePatt(int patno);

	int arMultiActivate(ARMultiMarkerInfoT *config);

	int arMultiDeactivate( ARMultiMarkerInfoT *config );

	int verify_markers(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

	int arInitCparam( Camera *pCam );

	int arGetLine(int x_coord[], int y_coord[], int coord_num, int vertex[], ARFloat line[4][3], ARFloat v[4][2]);

	int arGetLine2(int x_coord[], int y_coord[], int coord_num, int vertex[], ARFloat line[4][3], ARFloat v[4][2], Camera *pCam);

	static int arUtilMatMul(ARFloat s1[3][4], ARFloat s2[3][4], ARFloat d[3][4]);

	static int arUtilMatInv(ARFloat s[3][4], ARFloat d[3][4]);

	static int arMatrixPCA(ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean);

	static int arMatrixPCA2(ARMat *input, ARMat *evec, ARVec *ev);

	static int arParamSaveDouble(char *filename, int num, ARParamDouble *param, ...);

	static int arParamLoadDouble(char *filename, int num, ARParamDouble *param, ...);

	static int arParamDecomp(ARParam *source, ARParam *icpara, ARFloat trans[3][4]);

	static int arParamDecompMat(ARFloat source[3][4], ARFloat cpara[3][4], ARFloat trans[3][4]);

	int arParamObserv2Ideal_none(Camera* pCam, ARFloat ox, ARFloat oy, ARFloat *ix, ARFloat *iy);

	int arParamObserv2Ideal_LUT(Camera* pCam, ARFloat ox, ARFloat oy, ARFloat *ix, ARFloat *iy);

	int arParamObserv2Ideal_std(Camera* pCam, ARFloat ox, ARFloat oy, ARFloat *ix, ARFloat *iy);
	int arParamIdeal2Observ_std(Camera* pCam, ARFloat ix, ARFloat iy, ARFloat *ox, ARFloat *oy);

	typedef int (TrackerImpl::* ARPARAM_UNDIST_FUNC)(Camera* pCam, ARFloat ox, ARFloat oy, ARFloat *ix, ARFloat *iy);

	typedef ARFloat (TrackerImpl::* POSE_ESTIMATOR_FUNC)(ARMarkerInfo *marker_info, ARFloat center[2], ARFloat width, ARFloat conv[3][4]);
	typedef ARFloat (TrackerImpl::* MULTI_POSE_ESTIMATOR_FUNC)(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

	void buildUndistO2ITable(Camera* pCam);

	void checkRGB565LUT();

	// calculates amount of data that will be allocated via artkp_Alloc()
	static size_t getDynamicMemoryRequirements();

	Profiler& getProfiler()  {  return profiler;  }


public:
	static void* operator new(size_t size);

	static void operator delete(void *rawMemory);


	// required for calib camera, should otherwise not be used directly
	//
	void setFittingMode(int nWhich)  {  arFittingMode = nWhich;  }

	ARFloat arGetTransMat3(ARFloat rot[3][3], ARFloat ppos2d[][2],
						   ARFloat ppos3d[][2], int num, ARFloat conv[3][4],
						   Camera *pCam);

	static int arParamObserv2Ideal(Camera *pCam, ARFloat ox, ARFloat oy, ARFloat *ix, ARFloat *iy);
	static int arParamIdeal2Observ(Camera *pCam, ARFloat ix, ARFloat iy, ARFloat *ox, ARFloat *oy);


protected:
	//int refineCorner(ARFloat &edge_x, ARFloat &edge_y, const ARFloat old_edge_x, const ARFloat old_edge_y, const int roi_radius,
	//				 const void* img_ptr, const int img_width, const int img_height);

	struct AutoThreshold {
		enum {
			MINLUM0 = 255,
			MAXLUM0 = 0
		};

		void reset()
		{
			minLum = MINLUM0;  maxLum = MAXLUM0;
		}

		void addValue(int nRed, int nGreen, int nBlue, int nPixelFormat)
		{
			int lum;

			// in RGB565 and LUM8 all three values are simply the grey value...
			if(nPixelFormat==PIXEL_FORMAT_RGB565 || nPixelFormat==PIXEL_FORMAT_LUM)
				lum = nRed;
			else
				lum = (nRed + (nGreen<<1) + nBlue)>>2;

			if(lum<minLum)
				minLum = lum;
			if(lum>maxLum)
				maxLum = lum;
		}

		int calc()
		{
			return (minLum+maxLum)/2;
		}

		bool enable;
		int minLum,maxLum;
		int numRandomRetries;
	} autoThreshold;


	PIXEL_FORMAT			pixelFormat;
	int						pixelSize;

	int						binaryMarkerThreshold;

	// arDetectMarker.cpp
	//
	ARMarkerInfo2			*marker_info2;
	ARMarkerInfo			*wmarker_info;
	int						wmarker_num;

	arPrevInfo				prev_info[MAX_IMAGE_PATTERNS];
	int						prev_num;

	arPrevInfo				sprev_info[2][MAX_IMAGE_PATTERNS];
	int						sprev_num[2];

	// arDetectMarker2.cpp
	//
	ARMarkerInfo2			*marker_infoTWO;		// CAUTION: this member has to be manually allocated!
													//          see TrackerSingleMarker for more info on this.

    int						arGetContour_wx[AR_CHAIN_MAX];
    int						arGetContour_wy[AR_CHAIN_MAX];


	// arGetCode.cpp
	int    pattern_num;
	int    patf[MAX_LOAD_PATTERNS];
	int    pat[MAX_LOAD_PATTERNS][4][PATTERN_HEIGHT*PATTERN_WIDTH*3];
	ARFloat patpow[MAX_LOAD_PATTERNS][4];
	int    patBW[MAX_LOAD_PATTERNS][4][PATTERN_HEIGHT*PATTERN_WIDTH*3];
	ARFloat patpowBW[MAX_LOAD_PATTERNS][4];

	ARFloat evec[EVEC_MAX][PATTERN_HEIGHT*PATTERN_WIDTH*3];
	ARFloat epat[MAX_LOAD_PATTERNS][4][EVEC_MAX];
	int    evec_dim;
	int    evecf;
	ARFloat evecBW[EVEC_MAX][PATTERN_HEIGHT*PATTERN_WIDTH*3];
	ARFloat epatBW[MAX_LOAD_PATTERNS][4][EVEC_MAX];
	int    evec_dimBW;
	int    evecBWf;

	// arGetMarkerInfo.cpp
	//
	ARMarkerInfo    marker_infoL[MAX_IMAGE_PATTERNS];

	// arGetTransMat.cpp
	//
	ARFloat  pos2d[P_MAX][2];
	ARFloat  pos3d[P_MAX][3];

	// arLabeling.cpp
	//
	int16_t      *l_imageL; //[HARDCODED_BUFFER_WIDTH*HARDCODED_BUFFER_HEIGHT];		// dyna
	int16_t      *l_imageR;
	int			 l_imageL_size;

	int          *workL;  //[WORK_SIZE];											// dyna
	int          *work2L; //[WORK_SIZE*7];											// dyna

	int          *workR;
	int          *work2R;
	int          *wareaR;
	int          *wclipR;
	ARFloat      *wposR;

	int          wlabel_numL;
	int          wlabel_numR;
	int          *wareaL;  //[WORK_SIZE];											// dyna
	int          *wclipL;  //[WORK_SIZE*4];											// dyna
	ARFloat       *wposL;  //[WORK_SIZE*2];											// dyna

	int        arFittingMode;
	int        arImageProcMode;
	Camera	   *arCamera;
	bool		loadCachedUndist;
	int        arImXsize, arImYsize;
	int        arTemplateMatchingMode;
	int        arMatchingPCAMode;

	uint8_t*   arImageL;

	MARKER_MODE		markerMode;

	unsigned char *RGB565_to_LUM8_LUT;		// lookup table for RGB565 to LUM8 conversion


	// camera distortion addon by Daniel
	//
	UNDIST_MODE		undistMode;
	unsigned int	*undistO2ITable;

	// used for Hull Tracking
	MarkerPoint	hullInPoints[MAX_HULL_POINTS];
	MarkerPoint	hullOutPoints[MAX_HULL_POINTS];

	CornerPoints	trackedCorners;

	ARFloat			relBorderWidth;

	ARPARAM_UNDIST_FUNC arParamObserv2Ideal_func;
	//ARPARAM_UNDIST_FUNC arParamIdeal2Observ_func;

	// RPP integration -- [t.pintaric]
	POSE_ESTIMATOR  poseEstimator;

    HULL_TRACKING_MODE hullTrackingMode;

	ARToolKitPlus::Logger	*logger;

	static int						screenWidth, screenHeight;
	int						thresh;

	ARParam					cparam;

	ARFloat					gl_para[16];
	ARFloat					gl_cpara[16];

	char					*descriptionString;

	struct {
		bool enabled;
		int corners, leftright, bottomtop;
	} vignetting;

#ifdef DEBUG_DIV_RANGE
	struct DBG_INFO {
		DBG_INFO() : hMin(30000<<16), hMax(-30000<<16), hxMin(30000<<16), hxMax(-30000<<16), hyMin(30000<<16), hyMax(-30000<<16), dxMax(0), dyMax(0)
		{}

		int hMin,hMax;
		int hxMin,hxMax;
		int hyMin,hyMax;
		int dxMax, dyMax;
	} dbgInfo;
#endif

	unsigned short			*DIV_TABLE;

	BCH						*bchProcessor;
	Profiler				profiler;
};


}	// namespace ARToolKitPlus



// this is templated code, so we need to include all this here...
//
#include <ARToolKitPlus_impl/core/arBitFieldPattern.cpp>
#include <ARToolKitPlus_impl/core/arDetectMarker.cpp>
#include <ARToolKitPlus_impl/core/arDetectMarker2.cpp>
#include <ARToolKitPlus_impl/core/arGetCode.cpp>
#include <ARToolKitPlus_impl/core/arGetMarkerInfo.cpp>
#include <ARToolKitPlus_impl/core/arGetTransMat.cpp>
#include <ARToolKitPlus_impl/core/arGetTransMat2.cpp>
#include <ARToolKitPlus_impl/core/arGetTransMat3.cpp>
#include <ARToolKitPlus_impl/core/rppGetTransMat.cpp> // RPP integration -- [t.pintaric]
#include <ARToolKitPlus_impl/core/arGetTransMatCont.cpp>
#include <ARToolKitPlus_impl/core/arLabeling.cpp>
#include <ARToolKitPlus_impl/core/arMultiActivate.cpp>
#include <ARToolKitPlus_impl/core/arMultiGetTransMat.cpp>
#include <ARToolKitPlus_impl/core/rppMultiGetTransMat.cpp> 	// RPP integration -- [t.pintaric]
#include <ARToolKitPlus_impl/core/arMultiReadConfigFile.cpp>
#include <ARToolKitPlus_impl/core/arUtil.cpp>
#include <ARToolKitPlus_impl/core/matrix.cpp>
#include <ARToolKitPlus_impl/core/mPCA.cpp>
#include <ARToolKitPlus_impl/core/paramDecomp.cpp>
#include <ARToolKitPlus_impl/core/paramDistortion.cpp>
#include <ARToolKitPlus_impl/core/paramFile.cpp>
#include <ARToolKitPlus_impl/core/vector.cpp>
#include <ARToolKitPlus_impl/core/arMultiGetTransMatHull.cpp>

#include <ARToolKitPlus_impl/arGetInitRot2.cpp>
#include <ARToolKitPlus_impl/TrackerImpl.cpp>

#endif //__ARTOOLKIT_TRACKERIMPL_HEADERFILE__
