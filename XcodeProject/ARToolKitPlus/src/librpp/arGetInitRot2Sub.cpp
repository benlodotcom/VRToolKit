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
 ** @author   Daniel Wagner
 *
 * $Id: arGetInitRot2Sub.cpp 180 2006-08-20 16:57:22Z daniel $
 * @file
 * ======================================================================== */


#include <ARToolKitPlus/arGetInitRot2Sub.h>
#include "rpp.h"
#include <cstring>
#include "rpp_vecmat.h"

namespace rpp {


void arGetInitRot2_sub2(real_t &err, mat33_t &R, vec3_t &t, const vec3_array &_model, const vec3_array &_iprts, const options_t _options);

void arGetInitRot2_sub(rpp_float &err,
					   rpp_mat &R,
					   rpp_vec &t,
					   const rpp_float cc[2],
					   const rpp_float fc[2],
					   const rpp_vec *model,
					   const rpp_vec *iprts,
					   const unsigned int model_iprts_size,
					   const rpp_mat R_init,
					   const bool estimate_R_init,
					   const rpp_float epsilon,
					   const rpp_float tolerance,
					   const unsigned int max_iterations)
{
	vec3_array _model;
	vec3_array _iprts;
	_model.resize(model_iprts_size);
	_iprts.resize(model_iprts_size);

	mat33_t K, K_inv;
	mat33_eye(K);
	K.m[0][0] = (real_t)fc[0];
	K.m[1][1] = (real_t)fc[1];
	K.m[0][2] = (real_t)cc[0];
	K.m[1][2] = (real_t)cc[1];

	mat33_inv(K_inv, K);

	for(unsigned int i=0; i<model_iprts_size; i++)
	{
		vec3_t _v,_v2;
		vec3_assign(_v,(real_t)model[i][0],(real_t)model[i][1],(real_t)model[i][2]);
		_model[i] = _v;
		vec3_assign(_v,(real_t)iprts[i][0],(real_t)iprts[i][1],(real_t)iprts[i][2]);
		vec3_mult(_v2,K_inv,_v);
		_iprts[i] = _v2;
	}

	options_t options;
	options.max_iter = max_iterations;
	options.epsilon = (real_t)(epsilon == 0 ? DEFAULT_EPSILON : epsilon);
	options.tol =     (real_t)(tolerance == 0 ? DEFAULT_TOL : tolerance);
	if(estimate_R_init)
		mat33_set_all_zeros(options.initR);
	else
	{
		mat33_assign(options.initR,
			(real_t)R_init[0][0], (real_t)R_init[0][1], (real_t)R_init[0][2],
			(real_t)R_init[1][0], (real_t)R_init[1][1], (real_t)R_init[1][2],
			(real_t)R_init[2][0], (real_t)R_init[2][1], (real_t)R_init[2][2]);
	}

	real_t _err;
	mat33_t _R;
	vec3_t _t;

	arGetInitRot2_sub2(_err,_R,_t,_model,_iprts,options);

	for(int j=0; j<3; j++)
	{
		R[j][0] = (rpp_float)_R.m[j][0];
		R[j][1] = (rpp_float)_R.m[j][1];
		R[j][2] = (rpp_float)_R.m[j][2];
		t[j] = (rpp_float)_t.v[j];
	}
	err = (rpp_float)_err;
}


void arGetInitRot2_sub2(real_t &err, mat33_t &R, vec3_t &t, const vec3_array &_model, const vec3_array &_iprts, const options_t _options)
{
	mat33_t Rlu_;
	vec3_t tlu_;
	unsigned int it1_;
	real_t obj_err1_;
	real_t img_err1_;

	vec3_array model(_model.begin(),_model.end());
	vec3_array iprts(_iprts.begin(),_iprts.end());
	options_t options;
	memcpy(&options,&_options,sizeof(options_t));

	mat33_clear(Rlu_);
	vec3_clear(tlu_);
	it1_ = 0;
	obj_err1_ = 0;
	img_err1_ = 0;

	objpose(R, t, it1_, obj_err1_, img_err1_, false, model, iprts, options);

	/*pose_vec sol;
	sol.clear();
	get2ndPose_Exact(sol,iprts,model,Rlu_,tlu_,0);
	int min_err_idx = (-1);
	real_t min_err = MAX_FLOAT;
	for(unsigned int i=0; i<sol.size(); i++)
	{
		mat33_copy(options.initR,sol[i].R);
		objpose(Rlu_, tlu_, it1_, obj_err1_, img_err1_, true, model, iprts, options);
		mat33_copy(sol[i].PoseLu_R,Rlu_);
		vec3_copy(sol[i].PoseLu_t,tlu_);
		sol[i].obj_err = obj_err1_;
		if(sol[i].obj_err < min_err)
		{
			min_err = sol[i].obj_err;
			min_err_idx = i;
		}
	}

	if(min_err_idx >= 0)
	{
		mat33_copy(R,sol[min_err_idx].PoseLu_R);
		vec3_copy(t,sol[min_err_idx].PoseLu_t);
		err = sol[min_err_idx].obj_err;
	}
	else
	{
		mat33_clear(R);
		vec3_clear(t);
		err = MAX_FLOAT;
	}*/
}

} // namespace rpp
