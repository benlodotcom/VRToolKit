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


#define ADMIN_MODE TRUE

#import "VRViewController.h"
#import "EAGLView.h"
#import "Object3D.h"

/*!
 Private interface for the VRViewController class
 */
@interface VRViewController ()

/*!
	@brief The ARToolKitPlusWrapper instance the controller pass on video frames for marler detection

	The VRViewController is also the delegate of this wrapper.
*/
@property (nonatomic, retain) ARToolKitPlusWrapper *wrapper;
/*!
	@brief The controller responsible of the parameters view
 */
@property (nonatomic, retain) Object3DParametersViewController *parametersViewController;
/*!
	@brief The capture session we use to capture video frames
 */	
@property (nonatomic, retain) AVCaptureSession *captureSession;
/*!
	@brief The layer displaying the frames from the camera
 */	
@property (nonatomic, retain) AVCaptureVideoPreviewLayer *previewLayer;
/*!
	/n
 */	
@property (nonatomic, assign) BOOL detectionEnabled;
/*!
	@brief The markerID of the current marker detect (-1 if or marker is detected)
 */	
@property (nonatomic, assign) int currentMarkerID;
/*!
	@brief The NSDictionnary loaded from the models.plist file.
 
	This dictionnary uses as key the markerID and as value the corresponding xml description
	file to load. 
 */	
@property (nonatomic, retain) NSDictionary *models;

/*!
	@brief	This methods initialize the capture session.
 
	It sets up the AVCaptureDeviceInput instance, the AVCaptureVideoDataOutput instance and create an AVCaptureSession
	which links the input and the output.\n 
	The format chosen for the capture is kCVPixelFormatType_32BGRA because for now ARToolKitPlusWrapper want BGRA encoded frames.\n
	The queue used is not the main queue, so all the capture and detection
	process is done in a separate thread.
*/
- (void)initCapture;

/*!
 @brief	This shows the parameters view
 */
- (void)hideParameters;
/*!
 @brief	This hides the parameters view
 */
- (void)showParameters;

@end


@implementation VRViewController

@synthesize wrapper = _wrapper;
@synthesize overlay = _overlay;
@synthesize parametersViewController = _parametersViewController;
@synthesize captureSession = _captureSession;
@synthesize previewLayer = _previewLayer;
@synthesize detectionEnabled = _detectionEnabled;
@synthesize currentMarkerID = _currentMarkerID;
@synthesize models = _models;
@synthesize parametersButton = _parametersButton;


#pragma mark -
#pragma mark Initialization
- (void)viewDidLoad {
	[super viewDidLoad];
	/*We hide the parameter button if the admin mode is disabled*/
	if (!ADMIN_MODE) self.parametersButton.hidden = TRUE;
	/*We initialize some variables*/
	self.currentMarkerID = -1;
	/*We load the models dictionnarry to get the correspondance between markers and models*/
	self.models = [NSDictionary dictionaryWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"models" ofType:@"plist"]];
	/*We start the capture*/
	[self initCapture];
}

- (void)initCapture {
	/*We setup the input*/
	AVCaptureDeviceInput *captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] 
										  error:nil];
	/*We setupt the output*/
	AVCaptureVideoDataOutput *captureOutput = [[AVCaptureVideoDataOutput alloc] init]; 
	captureOutput.alwaysDiscardsLateVideoFrames = YES;
	/*For now we use the main queue, it might be better to use a separate queue but it involves some work as drawing and other
	 operations must be done in the main thread*/
	dispatch_queue_t queue;
	queue = dispatch_queue_create("cameraQueue", NULL);
	[captureOutput setSampleBufferDelegate:self queue:queue];
	dispatch_release(queue);
	/*Set the video output to store frame in BGRA (BGRA is supposed to be more performant)*/
	NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
	NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA]; 
	NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key]; 
	[captureOutput setVideoSettings:videoSettings]; 
	/*And we create a capture session*/
	self.captureSession = [[AVCaptureSession alloc] init];
	[self.captureSession setSessionPreset:AVCaptureSessionPresetMedium];
	/*We add input and output*/
	[self.captureSession addInput:captureInput];
	[self.captureSession addOutput:captureOutput];
	/*We add the previewLayer*/
	self.previewLayer = [AVCaptureVideoPreviewLayer layerWithSession: self.captureSession];
	self.previewLayer.frame = self.view.bounds;
	self.previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
	self.previewLayer.zPosition = -5;
	[self.view.layer addSublayer:self.previewLayer];
}
	
- (void)viewWillAppear:(BOOL)animated {
	if (!self.detectionEnabled) {
		[self startDetection];
	}
}
#pragma mark -
#pragma mark start stop detection

- (void)startDetection {
	/*We start the capture*/
	[self.captureSession startRunning];
	/*We allow the detection*/
	self.detectionEnabled = TRUE;
}

- (void)stopDetection {
	/*We stop the capture*/
	[self.captureSession stopRunning];
	/*We disable the detection*/
	self.detectionEnabled = FALSE;
}

#pragma mark -
#pragma mark AVCaptureSession delegate
- (void)captureOutput:(AVCaptureOutput *)captureOutput 
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
	   fromConnection:(AVCaptureConnection *)connection 
{ 
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	if (self.detectionEnabled) {
		if (!self.wrapper) {
			self.wrapper = [[ARToolKitPlusWrapper alloc] init];
			self.wrapper.delegate = self;
			[self.wrapper setupWithImageBuffer:imageBuffer];
		}
		[self.wrapper detectMarkerInImageBuffer:imageBuffer];
	}
}

#pragma mark -
#pragma mark ARToolKitPlusWrapperDelegate
- (void)aRToolKitPlusWrapper:(ARToolKitPlusWrapper *)wrapper didSetupWithProjectionMatrix:(NSArray *)projectionMatrix {
	[self.overlay performSelectorOnMainThread:@selector(setProjectionMatrix:) withObject:projectionMatrix waitUntilDone:YES];
}

- (void)aRToolKitPlusWrapper:(ARToolKitPlusWrapper *)wrapper didDetectMarker:(int)markerID withConfidence:(float)confidence
		andComputeModelViewMatrix:(NSArray *)modelViewMatrix 
{	
	/*We create an auto release pool for this thread*/
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	/*We switch the markerID*/
	int previousMarkerID = self.overlay.markerID;
	self.overlay.markerID = markerID;
	
	/*If a marker has been detected*/
	if (markerID != -1) {
		
		/*If this ID is different from the previous one we load the corresponding model*/
		if (markerID!=previousMarkerID) {
			/*We load the object corresponding to the markerID*/
			NSString *objectFileName = (NSString *) [self.models objectForKey:[NSString stringWithFormat:@"%d",markerID]];
			Object3D *object = [Object3D object3DFromFileNamed:objectFileName];
			[self.overlay performSelectorOnMainThread:@selector(loadObject:) withObject:object waitUntilDone:YES];
			
			NSLog(@"Object loaded : \n%@", [object description]);
			
			/*If parameters are shown we change its display*/
			if (self.parametersViewController) [self.parametersViewController performSelectorOnMainThread:@selector(refreshView) withObject:nil waitUntilDone:YES];
		}
		
		/*We set the modelViewMatrix (has as effect to redraw the view)*/
		[self.overlay performSelectorOnMainThread:@selector(setModelViewMatrix:) withObject:modelViewMatrix waitUntilDone:YES];
		
		/*If the overlay is not displayed we display it*/
		if(!self.overlay.superview) {
			[self.view performSelectorOnMainThread:@selector(addSubview:) withObject:self.overlay waitUntilDone:YES];
			[self.view performSelectorOnMainThread:@selector(sendSubviewToBack:) withObject:self.overlay waitUntilDone:YES];
		}
	}
	else {
		
		if (markerID!=previousMarkerID) {
			/*If parameters are shown we change its display*/
			if (self.parametersViewController) [self.parametersViewController performSelectorOnMainThread:@selector(refreshView) withObject:nil waitUntilDone:YES];
		
			/*If the overlay is displayed we hide it*/
			if (self.overlay.superview) [self.overlay performSelectorOnMainThread:@selector(removeFromSuperview) 
																   withObject:nil waitUntilDone:YES];
		}
	}

	/*We release the pool*/
	[pool drain];
	
}

#pragma mark -
#pragma mark parameters
- (IBAction)showHideParameters:(id)sender {
	UIButton *button = (UIButton *) sender;
	if (self.parametersViewController) {
		/*We set the button state*/
		button.alpha = 0.5;
		/*We hide the parameters view*/
		[self hideParameters];
	}
	else {
		/*We set the button state*/
		button.alpha = 1.0;
		/*We hide the parameters view*/
		[self showParameters];
	}

}

- (void) showParameters {
	/*We display it*/
	self.parametersViewController = [[Object3DParametersViewController alloc] init];
	[self.parametersViewController release];
	self.parametersViewController.delegate = self;
	self.parametersViewController.glView = self.overlay;
	[self.view addSubview:self.parametersViewController.view];
}

- (void) hideParameters {
	[self.parametersViewController.view removeFromSuperview];
	self.parametersViewController = nil;
}

#pragma mark -
#pragma mark ParametersDelegate
- (void)object3DParametersViewControllerWillDisplayModalViewController:(Object3DParametersViewController*)controller {
	[self stopDetection];
}

- (void)object3DParametersViewControllerWillDismissModalViewController:(Object3DParametersViewController*)controller {
	[self startDetection];
}

#pragma mark -
#pragma mark Animation
- (IBAction)startStopAnimatingObject:(id)sender {
	UIButton *button = (UIButton *) sender;
	if (button.selected) {
		button.alpha = 0.5;
		[self.overlay stopAnimating];
	}
	else {
		button.alpha = 1.0;
		[self.overlay startAnimating];
	}
	button.selected = !button.selected;

}

#pragma mark -
#pragma mark Memory management

- (void)viewWillDisappear:(BOOL)animated {
	[self stopDetection];
}

- (void)viewDidUnload {
	self.overlay = nil;
	self.previewLayer = nil;
}

- (void)dealloc {
	[self.captureSession release];
	[self.parametersViewController release];
    [super dealloc];
}


@end
