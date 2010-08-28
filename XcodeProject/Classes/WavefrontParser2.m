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

#import "WavefrontParser2.h"

/*Private interface*/
@interface  WavefrontParser2 ()

+ (void)centerGLfloatArray:(GLfloat *)array numberOfFaces:(int)numberOfFaces;
+ (GLfloat *)gLfloatArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray;
+ (GLfloat *)gLTexturesArrayFromArray:(NSArray *)array usingIndexArray:(NSArray *)indexArray;

@end

@implementation WavefrontParser2

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
	
	//We load the wavefrontfile into a big NSString
	NSString *fileData = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:filename ofType:nil] 
												   encoding:NSASCIIStringEncoding error:nil];
	//We create an array where each element is a line of the file					
	NSArray *lines = [fileData componentsSeparatedByString:@"\n"];
	
	unsigned int numberOfFaces = 0;
	unsigned int numberOfVertices = 0;
	unsigned int numberOfNormals = 0;
	
	for (NSString *line in lines) {
		if ([line hasPrefix:@"f"]) {
			numberOfFaces++;
		}
	}
	
	
	//We create the arrays we will use and the counter for the number of vertices
	NSMutableArray *vertices = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	NSMutableArray *textures = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	NSMutableArray *normals = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	NSMutableArray *faceVerticesIndices = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	NSMutableArray *faceTexturesIndices = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	NSMutableArray *faceNormalsIndices = [NSMutableArray arrayWithCapacity:3*numberOfFaces];
	
	
	for (NSString *line in lines) {
		
		//We parse normals and store an array containing [x, y, z]
		if ([line hasPrefix:@"v"]) {
			NSArray *lineComponents = [line componentsSeparatedByString:@" "];
			//[normals addObject:[lineComponents subarrayWithRange:NSMakeRange(1,3)]];
		}
		//We parse textures and store an array containing [x, y, z]
		else if ([line hasPrefix:@"vt"]) {
			NSArray *lineComponents = [line componentsSeparatedByString:@" "];
			//[textures addObject:[lineComponents subarrayWithRange:NSMakeRange(1, 2)]];
		}
		//We parse vertices and store an array containing [x, y, z]
		else if ([line hasPrefix:@"vn"]) {
			NSArray *lineComponents = [line componentsSeparatedByString:@" "];
			//[vertices addObject:[lineComponents subarrayWithRange:NSMakeRange(1,3)]];
		}
		//We parse faces
		else if ([line hasPrefix:@"f"]) {
			NSArray *lineComponents = [line componentsSeparatedByString:@" "];
			for (int i=1; i<4; i++) {
				NSArray *faceIndexes = [[lineComponents objectAtIndex:i] componentsSeparatedByString:@"/"];
				if ([faceIndexes count] > 0) {
					//[faceVerticesIndices addObject:[faceIndexes objectAtIndex:0]];
					if ([faceIndexes count] > 1 && _containTextures) {
						
						NSString *textureIndex = [faceIndexes objectAtIndex:1];
						/*if ([textureIndex isEqualToString:@""]) {
							_containTextures = FALSE;
						}
						else [faceTexturesIndices addObject:textureIndex];*/
						
						if ([faceIndexes count] > 2 && _containNormals) {
							
							/*NSString *normalIndex = [faceIndexes objectAtIndex:2];
							if ([normalIndex isEqualToString:@""]) {
								_containNormals = FALSE;
							}
							else [faceNormalsIndices addObject:normalIndex];*/
							
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
	//glVertices = [WavefrontParser2 gLfloatArrayFromArray:vertices usingIndexArray:faceVerticesIndices];
	//[WavefrontParser2 centerGLfloatArray:glVertices numberOfFaces:[faceVerticesIndices count]];
	//We transform the normals data in something understandable by openGL 
	//if([normals count]>0) glNormals = [WavefrontParser2 gLfloatArrayFromArray:normals usingIndexArray:faceNormalsIndices];
	//We transform the textures data in something understandable by openGL 
	//if([textures count]>0) glTextures = [WavefrontParser2 gLTexturesArrayFromArray:textures usingIndexArray:faceTexturesIndices];
	
	//[self.delegate parserDidCountVertices:3*numberOfFaces andDidParseVertices:glVertices
	//							  normals:glNormals andTextures:glTextures];
	
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
