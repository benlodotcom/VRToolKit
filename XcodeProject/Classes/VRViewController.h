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


#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>
#import <QuartzCore/QuartzCore.h>
#import "ARToolKitPlusWrapper.h"
#import "Object3DParametersViewController.h"

@class EAGLView;
@class Object3DParametersViewController;
@class Object3D;

/*!
 @author Benjamin Loulier
 
 @brief This class is the core of the Application. 
 
 It initializes the capture of frames from the camera, send them to the ARToolKitPlusWrapper, 
 receives the result of the marker detection and pass on those results to the EAGLView.
 */

@interface VRViewController : UIViewController <AVCaptureVideoDataOutputSampleBufferDelegate, ARToolKitPlusWrapperDelegate, ParametersDelegate> {
@private
	ARToolKitPlusWrapper *_wrapper;
	EAGLView *_overlay;
	Object3DParametersViewController *_parametersViewController;
	AVCaptureSession *_captureSession;
	AVCaptureVideoPreviewLayer *_previewLayer;
	BOOL _detectionEnabled;
	int _currentMarkerID;
	NSDictionary *_models;
	UIButton *_parametersButton;
}

/*!
	@brief	The view where the 3D object is shown, this view is overlayed over 
	the VRViewController's view.
*/
@property (nonatomic, retain) IBOutlet EAGLView *overlay;
/*! \n */
@property (nonatomic, retain) IBOutlet UIButton *parametersButton;

/*!
	@brief	This method start the detection of markers in frames from the camera.
*/
- (void)startDetection;
/*!
	@brief	This method stop the detection of markers in frames from the camera.
 */
- (void)stopDetection;

/*!
	@brief	This method trigger the display of the parameters view.
 */
- (IBAction)showHideParameters:(id)sender;
/*!
	@brief	This method tell the EAGLView to animate the 3D Objects displayed
 */
- (IBAction)startStopAnimatingObject:(id)sender;

@end
