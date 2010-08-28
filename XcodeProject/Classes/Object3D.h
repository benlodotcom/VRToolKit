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

#import <Foundation/Foundation.h>
#import <OpenGLES/ES1/gl.h>
#import "Parser3DProtocol.h"


/*!
 @author Benjamin Loulier
 
 @brief	This class is the internal representation of an object you want to display in Virtual Reality
 
 This class is mainly a data container, but it also contains the methods to unserialize/serialize an
 object from/to its xml representation.
 */

@interface Object3D : NSObject <Parser3DDelegate> {
	@private
	NSString *_name;
	NSString *_xmlFilePath;
	float _scaleFactor;
	float _xRotation;
	float _yRotation;
	float _zRotation;
	float _zTranslation;
	NSString *_textureFileName;
	NSString *_modelFileName;
	//3D parameters
	unsigned int _numberOfVertices;
	GLfloat *_vertices;
	GLfloat *_normals;
	GLfloat *_textureCoordinates;
}

/*!
	@brief	The name of the 3D object
*/
@property (readonly,nonatomic, retain) NSString *name;
/*!
	@brief	The path of the xml file corresponding to this object
 */
@property (readonly, nonatomic, retain) NSString *xmlFilePath;
/*!
 @brief	The scaling applied when the object is displayed
 */
@property (nonatomic, assign) float scaleFactor;
/*!
 @brief	The rotation around the X axis applied when the object is displayed
 */
@property (nonatomic, assign) float xRotation;
/*!
 @brief	The rotation around the Y axis applied when the object is displayed
 */
@property (nonatomic, assign) float yRotation;
/*!
 @brief	The rotation around the Z axis applied when the object is displayed
 */
@property (nonatomic, assign) float zRotation;
/*!
 @brief	The translation along the Z axis applied when the object is displayed
 
 You can use this to stick the object to the marker or in the contrary to make it "float over"
 */
@property (nonatomic, assign) float zTranslation;
/*!
 @brief	The name of the texture file with its extension (only one texture file supported for now)
 */
@property (readonly, nonatomic, retain) NSString *textureFileName;
/*!
 @brief	The name of the shape description file (for now only .obj are supported and their parsing is damn slow)
 */
@property (readonly, nonatomic, retain) NSString *modelFileName;
//3D parameters
/*!
 @brief	The number of vertices of the object
 */
@property (readonly, nonatomic, assign) unsigned int numberOfVertices;
/*!
 @brief	The array containing all the vertices in the right order (ordered according to the faces)
 */
@property (readonly, nonatomic, assign) GLfloat *vertices;
/*!
 @brief	The array containing all the normals in the right order (ordered according to the faces)
 */
@property (readonly, nonatomic, assign) GLfloat *normals;
/*!
 @brief	The array containing all the texture coordinates in the right order (ordered according to the faces)
 */
@property (readonly, nonatomic, assign) GLfloat *textureCoordinates;

/*!
	@brief	This method unserialize an object from its xml representation
	
	This methods fill the object with the right datas coming from the xml,
 then it fills the array used to describe the shape of the 3D model and the texture coordinates.
 To do so it either parse the file with modelFileName or use an imported .h file.
 
	@param	filename The name with the extension of the file used for unserialization
	@return	An autoreleased instance of Object3D, that you can pass on to EAGLView for display
*/
+ (Object3D *)object3DFromFileNamed:(NSString *)filename;
/*!
	@brief	This method serialize and object and save its xml representation in the document directory of the app
*/
- (void)writeInDocumentDirectory;

@end
