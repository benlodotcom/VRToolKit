/* ========================================================================
 * PROJECT: ARToolKitPlus
 * ========================================================================
 * This work is based on the original ARToolKit developed by
 *   Hirokazu Kato
 *   Mark Billinghurst
 *   HITLab, University of Washington, Seattle
 * http://www.hitl.washington.edu/artoolkit/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2006 Graz University of Technology
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Dieter Schmalstieg
 *   <schmalstieg@icg.tu-graz.ac.at>
 *   Graz University of Technology, 
 *   Institut for Computer Graphics and Vision,
 *   Inffeldgasse 16a, 8010 Graz, Austria.
 * ========================================================================
 ** @author   Thomas Pintaric
 *
 * $Id: byteSwap.cxx 162 2006-04-19 21:28:10Z grabner $
 * @file
 * ======================================================================== */


#include <ARToolKitPlus/param.h>
#include <stdint.h>

namespace ARToolKitPlus {

#ifdef AR_LITTLE_ENDIAN

static void
byteSwapInt( int *from, int *to )
{
    uint8_t   *w1, *w2;

    w1 = (uint8_t *)from;
    w2 = (uint8_t *)to;
    
    for(int i = 0; i < 4; i++ ) {
        w2[i] = w1[3-i];
    }
}

// FIXME: This function is inheretly broken
// one must be breindead to try byte shuffling floating point numbers
static void
byteSwapDouble( double *from, double *to )
{
    uint8_t   *w1, *w2;

    w1 = (uint8_t *)from;
    w2 = (uint8_t *)to;
    
    for(int i = 0; i < 8; i++ ) {
        w2[i] = w1[7-i];
    }
}

void
byteswap( ARParamDouble *param )
{
    ARParamDouble  wparam;
    int            i, j;

    byteSwapInt( &(param->xsize), &(wparam.xsize) );
    byteSwapInt( &(param->ysize), &(wparam.ysize) );

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            byteSwapDouble( &(param->mat[j][i]), &(wparam.mat[j][i]) );
        }
    }

    for( i = 0; i < 4; i++ ) {
        byteSwapDouble( &(param->dist_factor[i]), &(wparam.dist_factor[i]) );
    }

    *param = wparam;
}

#endif  //AR_LITTLE_ENDIAN

}  // namespace ARToolKitPlus
