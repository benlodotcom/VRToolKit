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
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

@class Object3D;

/*!
 @author Benjamin Loulier
 
 @brief	This view is used to display 3D models using OpenGL ES
 
 This class handles the rendering of the 3D models, but also th
 */

@interface EAGLView : UIView
{
@private
	float *_projectionMatrix;
	float *_modelViewMatrix;
	/*The pixel dimensions of the backbuffer*/
	GLint backingWidth;
	GLint backingHeight;
	
	EAGLContext *context;
	
	/*OpenGL names for the renderbuffer and framebuffers used to render to this view*/
	GLuint viewRenderbuffer, viewFramebuffer;
	
	/*OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist)*/
	GLuint depthRenderbuffer;
	
	GLuint textures[1];
	
	/*The 3DObject that we draw on the view and the current markerID*/
	Object3D *_object;
	int _markerID;
	
	/*Animating*/
	BOOL animating;
	
	
}

/*!
 @brief The Object3D instance displayed
 */
@property (nonatomic, retain) Object3D *object;
/*!
 @brief The current MarkerID corresponding to the object displayed (if no object displayed -1)
 */
@property (nonatomic, assign) int markerID;

/*!
 @brief	Custom setter for the modelViewMatrix, it has as additional effect to redraw the view
 */
- (void)setModelViewMatrix:(NSArray *)matrix;
/*!
 @brief	Custom setter for the projectionMatrix, it has as additional effect to setup and redraw the view
 */
- (void)setProjectionMatrix:(NSArray *)matrix;

/*!
	@brief	This method is called to reresh the display of the 3D model
*/
- (void)drawView;


/*!
 @brief	This method is called to start animating the object
 */
- (void)startAnimating;
/*!
 @brief	This method is called to stop animating the object
 */
- (void)stopAnimating;

@end
