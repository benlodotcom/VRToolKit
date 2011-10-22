/*
 Copyright 2010 Benjamin Loulier <http://www.benjaminloulier.com> <benlodotcom@gmail.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#import "ARToolKitPlusWrapper.h"
#include "ARToolKitPlus/TrackerSingleMarkerImpl.h"

/*!
 The Object from ARToolKitPlus library responsible for detecting markers
 */
ARToolKitPlus::TrackerSingleMarker *tracker;


@implementation ARToolKitPlusWrapper

/*A Loger we define in C++*/
class MyLogger : public ARToolKitPlus::Logger
{
    void artLog(const char* nStr)
    {
        NSLog(@"%s",nStr);
    }
};

@synthesize delegate = _delegate;
@synthesize useBCHMarker = _useBCHMarker;
@synthesize useThinBorderMarker = _useThinBorderMarker;

#pragma mark -
#pragma mark Initialization

- (id)init {
	self = [super init];
	if (self) {
		/*Initialization of parameters*/
		self.useBCHMarker = FALSE;
		self.useThinBorderMarker = FALSE;
	}
	return self;
}
- (void)setupWithImageBuffer:(CVImageBufferRef)imageBuffer {
	
	/*We lock the buffer and compute its width and height*/
	CVPixelBufferLockBaseAddress(imageBuffer,0);
	size_t width = CVPixelBufferGetWidth(imageBuffer);
	size_t height = CVPixelBufferGetHeight(imageBuffer);
	
    MyLogger      logger;
	
    /* create a tracker that does:
      - 6x6 sized marker images
      - samples at a maximum of 6x6
      - works with luminance (gray) images
      - can load a maximum of 1 pattern
      - can detect a maximum of 8 patterns in one image*/
    if(tracker) delete tracker;
	tracker = new ARToolKitPlus::TrackerSingleMarkerImpl<6,6,6, 1, 8>(width,height);
	
    /*We use a logger to output error messages*/
    tracker->setLogger(&logger);
	
	/*We set up the pixel format, it has to be the same than the one in your AVCaptureVideoDataoutput*/
	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_BGRA);
	
    /*We load the camera description file (not a specific one to the iPhone because no calibration has 
	 been done for now)*/
	NSString * path = [[NSBundle mainBundle] pathForResource:@"camera_para" ofType:@"dat"]; 
	tracker->init([path UTF8String], 0.1f, 10000.0f);
	tracker->changeCameraSize(width, height);
	
	/*We set the width of the border*/
    tracker->setBorderWidth((self.useThinBorderMarker||self.useBCHMarker) ? 0.125f : 0.250f);
	
	/*We use either the simple markers of the bch markers*/
    tracker->setMarkerMode(self.useBCHMarker ? ARToolKitPlus::MARKER_ID_BCH : ARToolKitPlus::MARKER_ID_SIMPLE);
	
    /*We use automatic thresholding, this ways if lighting conditions are changing the tracking is still accurate*/
	tracker->activateAutoThreshold(TRUE);
	
    /*let's use lookup-table undistortion for high-speed
    note: LUT only works with images up to 1024x1024*/
    tracker->setUndistortionMode(ARToolKitPlus::UNDIST_LUT);
	
    /*We use RPP pose estimator because it's more robust*/
    tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);
	
	
	/*We get the projection matrix from artoolkit, copy it in a float array and send it to our delegate*/
	//float projectionMatrix[16] = {0};
	//float *projectionMatrix = (float *) malloc(16*sizeof(float));
	NSMutableArray *projectionMatrix = [NSMutableArray arrayWithCapacity:16];
	for (int i=0; i<16; i++) {
		//projectionMatrix[i] = tracker->getProjectionMatrix()[i];
		[projectionMatrix addObject:[NSNumber numberWithFloat:tracker->getProjectionMatrix()[i]]];
	}
	
	/*We unlock the pixel buffer, we are done with it*/
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	
	/*We call our delegate*/
	[self.delegate aRToolKitPlusWrapper:self didSetupWithProjectionMatrix:projectionMatrix];
	
}

#pragma mark -
#pragma mark detection

- (void)detectMarkerInImageBuffer:(CVImageBufferRef)imageBuffer {
	
	/*We lock the buffer and get the address of the first pixel*/
	CVPixelBufferLockBaseAddress(imageBuffer,0);
	unsigned char *baseAddress = (unsigned char *) CVPixelBufferGetBaseAddress(imageBuffer);
	
	/*We get the identifier of the marer detected and the confidence value*/
    int markerId = tracker->calc(baseAddress);
    float conf = (float)tracker->getConfidence();
	
    /*We get the modelView matrix and copy it in a float array*/
	//float modelViewMatrix[16] = {0};
	NSMutableArray *modelViewMatrix = [NSMutableArray arrayWithCapacity:16];
	if (markerId!=-1) {
		int i;
		for (i=0; i<16; i++) {
			[modelViewMatrix addObject:[NSNumber numberWithFloat:tracker->getModelViewMatrix()[i]]];
		}
	}
	
	/*We unlock the pixel buffer, we are done with it*/
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	
	/*We call our delegate*/
	[self.delegate aRToolKitPlusWrapper:self didDetectMarker:markerId withConfidence:conf
				   andComputeModelViewMatrix:modelViewMatrix];
		
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc {
	if(tracker) delete tracker;
	[super dealloc];
}

@end
