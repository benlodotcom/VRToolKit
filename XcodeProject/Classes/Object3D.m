#import "Object3D.h"
#import "GDataXMLNode.h"
//We import all the preprocessed models
#import "banana.h"
#import "plane.h"
#import "apple.h"
#import "eiffel.h"
#import "cube.h"
//We import the parsers
#import "WavefrontParser.h"
#import "WavefrontParser2.h"


/*!
 Private interface of the Object3D class
 */
@interface Object3D ()

@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *xmlFilePath;
@property (nonatomic, retain) NSString *textureFileName;
@property (nonatomic, retain) NSString *modelFileName;
@property (nonatomic, assign) unsigned int numberOfVertices;
@property (nonatomic, assign) GLfloat *vertices;
@property (nonatomic, assign) GLfloat *normals;
@property (nonatomic, assign) GLfloat *textureCoordinates;

/*!
	@brief	This methods fills the numberOfVertices, vertices, normals and textureCoordinates
	with datas coming from either a .h file or a description file parsed by the appropriate parse

	You have to modify this function if you want to add objects whose shape is described in a .h file
*/
- (void)loadShapesAndTextures;

@end

@implementation Object3D

@synthesize name = _name;
@synthesize xmlFilePath = _xmlFilePath;
@synthesize scaleFactor = _scaleFactor;
@synthesize xRotation = _xRotation;
@synthesize yRotation = _yRotation;
@synthesize zRotation = _zRotation;
@synthesize zTranslation = _zTranslation;
@synthesize textureFileName = _textureFileName;
@synthesize modelFileName = _modelFileName;
//3D parameters
@synthesize numberOfVertices = _numberOfVertices;
@synthesize vertices = _vertices;
@synthesize normals = _normals;
@synthesize textureCoordinates = _textureCoordinates;

#pragma mark -
#pragma mark Initialization

- (id) init {
	self = [super init];
	if (self) {
		self.name = @"Default";
		self.scaleFactor = 1.0f;
		self.xRotation = 0.0f;
		self.yRotation = 0.0f;
		self.zRotation = 0.0f;
		self.zTranslation = 0.0f;
		self.textureFileName = @"Default";
		self.modelFileName = @"Default";
	}
	return self;
}


#pragma mark -
#pragma mark Serialization/Unserialization
+ (NSString *)applicationDocumentsDirectory {
	
    NSArray *paths = 
	NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
    return basePath;
}

+ (Object3D *)object3DFromFileNamed:(NSString *)filename {
	
	Object3D *object = [[[Object3D alloc] init] autorelease];
	
	/*We load the file from the document directory or from app directory*/
	object.xmlFilePath = [[Object3D applicationDocumentsDirectory] stringByAppendingPathComponent:filename];
	if (![[NSFileManager defaultManager] fileExistsAtPath:object.xmlFilePath]) {
		NSLog(@"Object not in document directory");
		object.xmlFilePath = [[NSBundle mainBundle] pathForResource:filename ofType:nil];
	}
	
	/*We read and parse the file (DOM way)*/
	NSError *error;
	NSData *xmlData = [NSData dataWithContentsOfFile:object.xmlFilePath];
	GDataXMLDocument *document = [[[GDataXMLDocument alloc] initWithData:xmlData 
																 options:0 error:&error] autorelease];
    if (document == nil) { return nil; }
	
	/*Then we bind the element to the properties of the object*/
	NSArray *names = [document.rootElement elementsForName:@"Name"];
	if (names.count > 0) {
		object.name = [(GDataXMLElement *)[names objectAtIndex:0] stringValue];
	} else {
		NSLog(@"No Name markup in the xml file.");
		return nil;
	}
	
	
	/*Display parameter*/
	NSArray *displayParameters = [document.rootElement elementsForName:@"DisplayParameters"];
	if (displayParameters.count > 0) {
		GDataXMLElement * displayParameter = (GDataXMLElement *) [displayParameters objectAtIndex:0];
		/*Scale factor*/
		NSArray *scaleFactors = [displayParameter elementsForName:@"ScaleFactor"];
		if (scaleFactors.count > 0) {
			object.scaleFactor = [[(GDataXMLElement *) [scaleFactors objectAtIndex:0] stringValue] floatValue];
		} else {
			NSLog(@"No ScaleFactor markup in the xml file.");
		}
		/*xRotation*/
		NSArray *xRotations = [displayParameter elementsForName:@"XRotation"];
		if (xRotations.count > 0) {
			object.xRotation = [[(GDataXMLElement *) [xRotations objectAtIndex:0] stringValue] floatValue];
		} else {
			NSLog(@"No XRotation markup in the xml file.");
		}
		/*yRotation*/
		NSArray *yRotations = [displayParameter elementsForName:@"YRotation"];
		if (yRotations.count > 0) {
			object.yRotation = [[(GDataXMLElement *) [yRotations objectAtIndex:0] stringValue] floatValue];
		} else {
			NSLog(@"No YRotation markup in the xml file.");
		}
		/*zRotation*/
		NSArray *zRotations = [displayParameter elementsForName:@"ZRotation"];
		if (zRotations.count > 0) {
			object.zRotation = [[(GDataXMLElement *) [zRotations objectAtIndex:0] stringValue] floatValue];
		} else {
			NSLog(@"No ZRotation markup in the xml file.");
		}
		/*zTranslation*/
		NSArray *zTranslations = [displayParameter elementsForName:@"ZTranslation"];
		if (zTranslations.count > 0) {
			object.zTranslation = [[(GDataXMLElement *) [zTranslations objectAtIndex:0] stringValue] floatValue];
		} else {
			NSLog(@"No ZTranslation markup in the xml file.");
		}
	}
	else {
		NSLog(@"No DisplayParameters markup in the xml file.");
	}
	
	/*Files*/
	NSArray *files = [document.rootElement elementsForName:@"Files"];
	if (files.count > 0) {
		GDataXMLElement *file = (GDataXMLElement *) [files objectAtIndex:0];
		
		/*Texture file*/
		NSArray *textureFileNames = [file elementsForName:@"TextureFileName"];
		if (textureFileNames.count > 0) {
			object.textureFileName = [(GDataXMLElement *)[textureFileNames objectAtIndex:0] stringValue];
		}
		else {
			NSLog(@"No TextureFileName markup in the xml file.");
		}
		
		/*Model file name*/
		NSArray *modelFileNames = [file elementsForName:@"ModelFileName"];
		if (modelFileNames.count > 0) {
			object.modelFileName = [(GDataXMLElement *)[modelFileNames objectAtIndex:0] stringValue];
		}
		else {
			NSLog(@"No ModelFileName markup in the xml file.");
		}
		
	} else {
		NSLog(@"No Files markup in the xml file");
	}
	
	//We load the shapes and textures description in memory
	[object loadShapesAndTextures];
	
	return object;
}

- (void)writeInDocumentDirectory {
	/*We create the root element*/
	GDataXMLElement *rootElement = [GDataXMLNode elementWithName:@"Object3D"];
	/*Then we create the children*/
	GDataXMLElement *name = [GDataXMLNode elementWithName:@"Name" stringValue:self.name];
	[rootElement addChild:name];
	
	/*Display parameter*/
	GDataXMLElement *displayParameters = [GDataXMLNode elementWithName:@"DisplayParameters"];
	
	GDataXMLElement *scaleFactor = [GDataXMLNode elementWithName:@"ScaleFactor" stringValue:[NSString stringWithFormat:@"%f", self.scaleFactor]];
	[displayParameters addChild:scaleFactor];
	GDataXMLElement *xRotation = [GDataXMLNode elementWithName:@"XRotation" stringValue:[NSString stringWithFormat:@"%f", self.xRotation]];
	[displayParameters addChild:xRotation];
	GDataXMLElement *yRotation = [GDataXMLNode elementWithName:@"YRotation" stringValue:[NSString stringWithFormat:@"%f", self.yRotation]];
	[displayParameters addChild:yRotation];
	GDataXMLElement *zRotation = [GDataXMLNode elementWithName:@"ZRotation" stringValue:[NSString stringWithFormat:@"%f", self.zRotation]];
	[displayParameters addChild:zRotation];
	GDataXMLElement *zTranslation = [GDataXMLNode elementWithName:@"ZTranslation" stringValue:[NSString stringWithFormat:@"%f", self.zTranslation]];
	[displayParameters addChild:zTranslation];
	
	[rootElement addChild:displayParameters];
	
	
	/*Files*/
	GDataXMLElement *files = [GDataXMLNode elementWithName:@"Files"];
	
	GDataXMLElement *textureFileName = [GDataXMLNode elementWithName:@"TextureFileName" stringValue:self.textureFileName];
	[files addChild:textureFileName];
	GDataXMLElement *modelFileName = [GDataXMLNode elementWithName:@"ModelFileName" stringValue:self.modelFileName];
	[files addChild:modelFileName];
	
	[rootElement addChild:files];
	
	/*We write the file*/
	GDataXMLDocument *document = [[GDataXMLDocument alloc] 
								  initWithRootElement:rootElement];
    NSData *xmlData = document.XMLData;
	self.xmlFilePath = [[Object3D applicationDocumentsDirectory] stringByAppendingPathComponent:[self.xmlFilePath lastPathComponent]];
	[xmlData writeToFile:self.xmlFilePath atomically:YES];
	[document release];
}
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

- (void)loadShapesAndTextures {
	//Objects loaded from an obj description file
	if ([[self.modelFileName pathExtension] isEqualToString:@"obj"]) {
		WavefrontParser *parser = [[WavefrontParser alloc] init];
		parser.delegate = self;
		[parser parseFileNamed:self.modelFileName];
		[parser release];
	}
	else if ([self.name isEqualToString:@"Banana"]) {
		self.numberOfVertices = bananaNumVerts;
		self.vertices = bananaVerts;
		self.normals = bananaNormals;
		self.textureCoordinates = bananaTexCoords;
	}
	else if ([self.name isEqualToString:@"Plane"]) {
		self.numberOfVertices = planeNumVerts;
		self.vertices = planeVerts;
		self.normals = planeNormals;
		self.textureCoordinates = NULL;
	}
	else if ([self.name isEqualToString:@"Apple"]) {
		self.numberOfVertices = appleNumVerts;
		self.vertices = appleVerts;
		self.normals = NULL;
		self.textureCoordinates = appleTexCoords;
	}
	else if ([self.name isEqualToString:@"Tour Eiffel"]) {
		self.numberOfVertices = eiffelNumVerts;
		self.vertices = eiffelVerts;
		self.normals = NULL;
		self.textureCoordinates = NULL;
	}

}

- (void) parserDidCountVertices:(unsigned int)numberOfVertices andDidParseVertices:(GLfloat *)vertices 
						normals:(GLfloat *)normals andTextures:(GLfloat *)textures;
{
	self.numberOfVertices = numberOfVertices;
	self.vertices = vertices;
	self.normals = normals;
	self.textureCoordinates = textures;
}

#pragma mark -
#pragma mark other
- (NSString *)description {
	NSMutableString *description = [[NSMutableString alloc] init];
	[description appendFormat:@"\nObject named: %@ \n", self.name];
	[description appendFormat:@"Display parameters: \n\t- scale:%f \n\t- xRot:%f \n\t- yRot:%f \n\t- zRot:%f\n", 
	 self.scaleFactor, self.xRotation, self.yRotation, self.zRotation];
	[description appendFormat:@"Files: \n\t- texture file: %@\n\t- model file name: %@\n",
	 self.textureFileName, self.modelFileName];
	[description appendFormat:@"Other: \n\t- Nb vertices: %d", self.numberOfVertices];
	return [description autorelease];
}

#pragma mark -
#pragma mark Memory managment
- (void)dealloc {
	[self.name release];
	[self.xmlFilePath release];
	[self.textureFileName release];
	[self.modelFileName release];
	[super dealloc];
}

@end
