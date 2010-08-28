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

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "EAGLView.h"
#import "Object3D.h"

/*!
 Private interface of EAGLView class
 */
@interface EAGLView ()

/*! \n */
- (BOOL)createFramebuffer;
/*! \n */
- (void)destroyFramebuffer;

/*!
 @brief This function initialize the OpenGL view
 
 This function sets up such things as lighting and the GL_Projection matrix. 
 This method is called when the projectionMatrix property is set.
 */
- (void)setupView;

/*!
 @brief This function loads the 3D model and textues associated to a 3DObject instance
 */
- (void)loadObject:(Object3D *)object;
/*!
 @brief This function loads the texture of the Object3D instance hold by the EAGLView
 */
- (void)loadTexture;
/*!
 @brief This function unloads the current texture
 */
- (void)unloadTexture;

@end

@implementation EAGLView

@synthesize object = _object;
@synthesize markerID = _markerID;

#pragma mark -
#pragma mark Custom getter/setter
- (void)setModelViewMatrix:(NSArray *)matrix {
	if (!_modelViewMatrix)  _modelViewMatrix = (float *) malloc([matrix count]*sizeof(float));
	int i = 0;
	for (NSNumber *number in matrix) {
		_modelViewMatrix[i] = [number floatValue];
		i++;
	}
	[self drawView];
}

- (void)setProjectionMatrix:(NSArray *)matrix {
	if (!_projectionMatrix)  _projectionMatrix = (float *) malloc([matrix count]*sizeof(float));
	int i = 0;
	for (NSNumber *number in matrix) {
		_projectionMatrix[i] = [number floatValue];
		i++;
	}
	[self setupView];
	[self drawView];
}

#pragma mark -
#pragma mark Initialization

/*Implement this to override the default layer class (which is [CALayer class]).
We do this so that our view will be backed by a layer that is capable of OpenGL ES rendering.*/
+ (Class) layerClass {
	return [CAEAGLLayer class];
}

// The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
	
    
    if ((self = [super initWithCoder:coder])) {
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		eaglLayer.opaque = NO;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		/*We apply transforms to the layer because the raw image of the camera is mirrored and rotated
		compared to what we see on the previewLayer. So to make the 3D object move the right way 
		regarding to our previewLayer we need to tweak a bit the layer.*/
		eaglLayer.contentsGravity = kCAGravityResizeAspectFill;
		eaglLayer.transform = CATransform3DRotate(CATransform3DIdentity, M_PI/2.0f, 0, 0, 1);
		eaglLayer.frame = self.bounds;
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		
		if (!context || ![EAGLContext setCurrentContext:context]) {
			[self release];
			return nil;
		}
		
	}
	
	/*We initialize some variables*/
	self.markerID = -1;
	
	return self;
}

#pragma mark -
#pragma mark Drawing

-(void)setupView {
	const GLfloat			lightAmbient[] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat			lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat			matAmbient[] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat			matDiffuse[] = {1.0, 1.0, 1.0, 1.0};	
	const GLfloat			matSpecular[] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat			lightPosition[] = {1.0, 1.0, 1.0, 1.0}; 
	const GLfloat			lightShininess = 100.0;
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, lightShininess);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition); 			
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	
	/*configure the projection*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(_projectionMatrix);
	glViewport(0, 0, self.bounds.size.width, self.bounds.size.height);
	
	//Make the OpenGL modelview matrix the default
	glMatrixMode(GL_MODELVIEW);
	
	//We enable normalization
	glEnable(GL_NORMALIZE);
}

- (void)loadObject:(Object3D *)object {
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	[self unloadTexture];
	
	self.object = object;
	/*configure arrays*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3 ,GL_FLOAT, 0, self.object.vertices);
	if (self.object.normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, self.object.normals);
	}
	if (self.object.textureCoordinates) {
		[self loadTexture];
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, self.object.textureCoordinates); 
	}
}

- (void) unloadTexture {
	glDeleteTextures(1, textures);
}

- (void) loadTexture {
	CGImageRef textureImage = [UIImage imageNamed:self.object.textureFileName].CGImage;
	if (textureImage == nil) {
		NSLog(@"The image for the texture has not been loaded");
		return;   
	}
	
	NSInteger textureWidth = CGImageGetWidth(textureImage);
	NSInteger textureHeight = CGImageGetHeight(textureImage);
	
	// un peu d'allocation dynamique de mémoire...
	GLubyte *textureData = (GLubyte *)malloc(textureWidth * textureHeight * 4); // 4 car RVBA
	
	CGContextRef textureContext = CGBitmapContextCreate(
														textureData,
														textureWidth,
														textureHeight,
														8, textureWidth * 4,
														CGImageGetColorSpace(textureImage),
														kCGImageAlphaPremultipliedLast);
	
	CGContextDrawImage(textureContext,
					   CGRectMake(0.0, 0.0, (float)textureWidth, (float)textureHeight),
					   textureImage);
	
	CGContextRelease(textureContext);
	
	glGenTextures(1,&textures[0]);
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	
	free(textureData);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);
}

// Updates the OpenGL view
static float spinZ = 0;

- (void)drawView
{
	if (_modelViewMatrix&&self.object) {
		// Make sure that you are drawing to the current context
		[EAGLContext setCurrentContext:context];
		
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Setup model view matrix
		glLoadIdentity();
		glLoadMatrixf(_modelViewMatrix);
		glScalef(self.object.scaleFactor, self.object.scaleFactor, self.object.scaleFactor);
		//User defined rotation
		glRotatef(self.object.zRotation + spinZ, 0.0, 0.0, 1.0);
		glRotatef(self.object.yRotation, 0.0, 1.0, 0.0);
		glRotatef(self.object.xRotation, 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.0, self.object.zTranslation);
		glRotatef(90.0f, 1.0, 0, 0);
		glTranslatef(0, 0.5, 0);
		// Draw teapot. The new_teapot_indicies array is an RLE (run-length encoded) version of the teapot_indices array in teapot.h
		glDrawArrays(GL_TRIANGLES, 0, self.object.numberOfVertices);
		
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
		[context presentRenderbuffer:GL_RENDERBUFFER_OES];
		
		if (animating) spinZ+=2;		
	}
}

- (void)startAnimating {
	animating = TRUE;
}

- (void)stopAnimating {
	animating = FALSE;
	spinZ = 0;
}

// If our view is resized, we'll be asked to layout subviews.
// This is the perfect opportunity to also update the framebuffer so that it is
// the same size as our display area.
-(void)layoutSubviews
{
	[EAGLContext setCurrentContext:context];
	[self destroyFramebuffer];
	[self createFramebuffer];
	[self drawView];
}

- (BOOL)createFramebuffer
{
	// Generate IDs for a framebuffer object and a color renderbuffer
	glGenFramebuffersOES(1, &viewFramebuffer);
	glGenRenderbuffersOES(1, &viewRenderbuffer);
	
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	// This call associates the storage for the current render buffer with the EAGLDrawable (our CAEAGLLayer)
	// allowing us to draw into a buffer that will later be rendered to screen wherever the layer is (which corresponds with our view).
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	// For this sample, we also need a depth buffer, so we'll create and attach one via another renderbuffer.
	glGenRenderbuffersOES(1, &depthRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	
	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}
	
	return YES;
}

#pragma mark -
#pragma mark Memory management

// Clean up any buffers we have allocated.
- (void)destroyFramebuffer
{
	glDeleteFramebuffersOES(1, &viewFramebuffer);
	viewFramebuffer = 0;
	glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	viewRenderbuffer = 0;
	
	if(depthRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &depthRenderbuffer);
		depthRenderbuffer = 0;
	}
}

- (void)dealloc
{
	
	if([EAGLContext currentContext] == context)
	{
		[EAGLContext setCurrentContext:nil];
	}
	
	free(_projectionMatrix);
	free(_modelViewMatrix);
	[context release];
	[self.object release];
	[super dealloc];
}

@end
