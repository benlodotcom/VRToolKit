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

#import "WavefrontParser.h"

/*!
 Private interface for the WavefrontParser class.
 */
@interface  WavefrontParser ()

/*!
	@brief	This method takes an array of vertices and translate them so that the shape represented is centered in (0,0)

	@param	array The array you want to center the vertices.
	@param	numberOfFaces The number of faces of the 3D object.
*/
+ (void)centerGLfloatArray:(GLfloat *)array numberOfFaces:(int)numberOfFaces;
/*!
	@brief	This method creates an array merging an index and some data

	For instance if the array is: 1,2,7,9,0 and the index array is: 0,0,4 the array returned will be
	1,1,9.\n
	This method is used to have an array of vertices corresponding to the faces of the 3D object.
	@param	array The array containing datas
	@param	indexArray The array containig the indexes
	@return	an array created from the datas and the index
*/
+ (GLfloat *)gLfloatArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray;
/*!
 @brief	This method creates an array merging an index and some data
 
 This method does the same job as (GLfloat *)gLfloatArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray
 but for the texture coordinate datas.\n
 We also do basic calculation to convert texture UV coordinates in ST coordinates so that openGL can understand them.
 This method is used to have an array of texture coordinates corresponding to the faces of the 3D object.
 
 @param	array The array containing datas
 @param	indexArray The array containig the indexes
 @return an array created from the datas and the index
 */
+ (GLfloat *)gLTexturesArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray;

@end

@implementation WavefrontParser

@synthesize delegate = _delegate;

#pragma mark -
#pragma mark Initialization

- (id)init {
	self = [super init];
	if (self) {
		_containNormals = TRUE;
		_containTextures = TRUE;
	}
	return self;
}


- (void) parseFileNamed:(NSString *)filename {
	
	//We create the arrays we will use and the counter for the number of vertices
	NSMutableArray *vertices = [NSMutableArray arrayWithCapacity:0];
	NSMutableArray *textures = [NSMutableArray arrayWithCapacity:0];
	NSMutableArray *normals = [NSMutableArray arrayWithCapacity:0];
	NSMutableArray *faceVerticesIndices = [NSMutableArray arrayWithCapacity:0];
	NSMutableArray *faceTexturesIndices = [NSMutableArray arrayWithCapacity:0];
	NSMutableArray *faceNormalsIndices = [NSMutableArray arrayWithCapacity:0];

	unsigned int numberOfFaces = 0;
	
	//We load the wavefrontfile into a big NSString
	NSString *fileData = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:filename ofType:nil] 
												   encoding:NSUTF8StringEncoding error:nil];
	//We create an array where each element is a line of the file					
	NSArray *lines = [fileData componentsSeparatedByString:@"\n"];
	
	for (NSString *line in lines) {
					  
		NSArray *lineComponents = [line componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
		
		//We parse normals and store an array containing [x, y, z]
		if ([(NSString *)[lineComponents objectAtIndex:0] isEqualToString:@"vn"]) {
			[normals addObject:[lineComponents subarrayWithRange:NSMakeRange(1,3)]];
		}
		//We parse textures and store an array containing [x, y, z]
		else if ([(NSString *)[lineComponents objectAtIndex:0] isEqualToString:@"vt"]) {
			[textures addObject:[lineComponents subarrayWithRange:NSMakeRange(1, 2)]];
		}
		//We parse vertices and store an array containing [x, y, z]
		else if ([(NSString *)[lineComponents objectAtIndex:0] isEqualToString:@"v"]) {
			[vertices addObject:[lineComponents subarrayWithRange:NSMakeRange(1,3)]];
		}
		//We parse faces
		else if ([(NSString *)[lineComponents objectAtIndex:0] isEqualToString:@"f"]) {
			numberOfFaces++;
			for (int i=1; i<4; i++) {
				NSArray *faceIndexes = [[lineComponents objectAtIndex:i] componentsSeparatedByString:@"/"];
				if ([faceIndexes count] > 0) {
					[faceVerticesIndices addObject:[faceIndexes objectAtIndex:0]];
					if ([faceIndexes count] > 1 && _containTextures) {
						
						NSString *textureIndex = [faceIndexes objectAtIndex:1];
						if ([textureIndex isEqualToString:@""]) {
							_containTextures = FALSE;
						}
						else [faceTexturesIndices addObject:textureIndex];
						
						if ([faceIndexes count] > 2 && _containNormals) {
							
							NSString *normalIndex = [faceIndexes objectAtIndex:2];
							if ([normalIndex isEqualToString:@""]) {
								_containNormals = FALSE;
							}
							else [faceNormalsIndices addObject:normalIndex];
							
						}
					}
				}
			}
		}

		
	}
	
	GLfloat *glVertices = NULL;
	GLfloat *glNormals = NULL;
	GLfloat *glTextures = NULL;
	
	//We transform the vertices data in something understandable by openGL 
	glVertices = [WavefrontParser gLfloatArrayFromArray:vertices usingIndexArray:faceVerticesIndices];
	[WavefrontParser centerGLfloatArray:glVertices numberOfFaces:[faceVerticesIndices count]];
	//We transform the normals data in something understandable by openGL 
	if([normals count]>0) glNormals = [WavefrontParser gLfloatArrayFromArray:normals usingIndexArray:faceNormalsIndices];
	//We transform the textures data in something understandable by openGL 
	if([textures count]>0) glTextures = [WavefrontParser gLTexturesArrayFromArray:textures usingIndexArray:faceTexturesIndices];
	
	[self.delegate parserDidCountVertices:3*numberOfFaces andDidParseVertices:glVertices
								  normals:glNormals andTextures:glTextures];
	
}


+ (GLfloat *)gLTexturesArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray {
	GLfloat *glArray = malloc([indexArray count]*2*sizeof(GLfloat));
	int i=0;
	for (NSString *index in indexArray) {
		int indice = [index intValue]-1;
		NSArray * coord = (NSArray *) [array objectAtIndex:indice];
		//We convert the UV coord in ST coord
		glArray[i++] = [(NSString *)[coord objectAtIndex:0] floatValue];
		glArray[i++] = 1 - [(NSString *)[coord objectAtIndex:1] floatValue];
	}
	return glArray;
}

+ (GLfloat *)gLfloatArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray {
	GLfloat *glArray = malloc([indexArray count]*3*sizeof(GLfloat));
	int i=0;
	for (NSString *index in indexArray) {
		int indice = [index intValue]-1;
		NSArray * coord = (NSArray *) [array objectAtIndex:indice];
		for (int j=0; j<3; j++) {
			glArray[i] = [(NSString *)[coord objectAtIndex:j] floatValue];
			i++;
		}
	}
	return glArray;
}

+ (void)centerGLfloatArray:(GLfloat *)array numberOfFaces:(int)numberOfFaces {
	GLfloat xSum = 0;
	GLfloat ySum = 0;
	GLfloat zSum = 0;
	
	for (int i=0; i<numberOfFaces*3; i+=3) {
		xSum+=array[i];
		ySum+=array[i+1];
		zSum+=array[i+2];
	}
	
	GLfloat xCenter = xSum/(float)numberOfFaces;
	GLfloat yCenter = ySum/(float)numberOfFaces;
	GLfloat zCenter = zSum/(float)numberOfFaces;
	
	for (int i=0; i<numberOfFaces*3; i+=3) {
		array[i]-=xCenter;
		array[i+1]-=yCenter;
		array[i+2]-=zCenter;
	}
}

@end
