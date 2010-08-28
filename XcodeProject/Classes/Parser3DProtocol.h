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

#import <OpenGLES/ES1/gl.h>

/*!
	@author Benjamin Loulier

	@brief	The protocol a class parsing 3D shape description files (.obj, .vrml ...) must implement.
 
	These parsers parse 3D format not the xml description file !
*/
@protocol Parser3DProtocol

/*!
 @brief	This method parse a file located in the mainBundle
 
 When the parsing is over the delegate is called.
 
 @param	filename The name of the file with the extension
 */
- (void) parseFileNamed:(NSString *)filename;

@end

/*!
 @author Benjamin Loulier
 
 @brief	The method the delegate of a parser must implement
 
 This protocols ensures that the parser can transmit datas to its delegate.
 */
@protocol Parser3DDelegate

/*!
	@brief	This method is called on the delegate when the parsing is over, it contains the arrays created while parsing the file.

	@param	numberOfVertices The number of vertices in the vertices array
	@param	vertices The array of vertices
	@param	normals The array of normals
	@param	textures The array of texture coordinates
*/
- (void) parserDidCountVertices:(unsigned int)numberOfVertices andDidParseVertices:(GLfloat *)vertices 
   normals:(GLfloat *)normals andTextures:(GLfloat *)textures;

@end