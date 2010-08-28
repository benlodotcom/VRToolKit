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
#import "Parser3DProtocol.h"

/*!
 @author Benjamin Loulier
 
 @brief This class can parse basic wavefront files (.obj)
 
 For now it is very slow, and is not very usable to load complicated objects on demand
 prefer the .h method in that case.
 */


@interface WavefrontParser : NSObject {
	@private
	id <Parser3DDelegate> _delegate;
	//Boolean iVar we use internally
	BOOL _containNormals;
	BOOL _containTextures;
}


/*!
	@brief	The object receiving the C arrays created while parsing.
*/
@property (nonatomic, assign) id <Parser3DDelegate> delegate;

/*!
 \n
 */
- (void) parseFileNamed:(NSString *)filename;

@end
