/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/math.h"

#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector3f32.h"

#include "graphics/opengl/system_headers.h"

namespace Tetraedge {

static inline uint _toGlLight(uint lightno) {
	return GL_LIGHT0 + lightno;
}

/*static*/
TeColor TeLight::_globalAmbientColor;

TeLight::TeLight() : _colAmbient(0, 0, 0, 0xff), _colDiffuse(0, 0, 0, 0xff), _colSpecular(0xff, 0xff, 0xff, 0xff),
_constAtten(1.0f), _linearAtten(0.0f), _quadraticAtten(0.0f), _cutoff(0.0f), _exponent(0.0f), _type(LightTypePoint),
_displaySize(3.0)
{
}

TeVector3f32 TeLight::directionVector() const {
	float cosx = cosf(_positionRadial.getX());
	float cosy = cosf(_positionRadial.getY());
	float sinx = sinf(_positionRadial.getX());
	float siny = sinf(_positionRadial.getY());
	return TeVector3f32(cosx * cosy, siny, sinx * cosy);
}

void TeLight::disable(uint lightno) {
	glDisable(_toGlLight(lightno));
}

void TeLight::enable(uint lightno) {
	if (_colDiffuse.r() == 0 && _colDiffuse.g() == 0 && _colDiffuse.b() == 0)
		glDisable(_toGlLight(lightno));
	else
		glEnable(_toGlLight(lightno));
}

/*static*/
void TeLight::disableAll() {
	glDisable(GL_LIGHTING);
}

/*static*/
void TeLight::enableAll() {
	glEnable(GL_LIGHTING);
}

void TeLight::draw(TeCamera &camera) {
	error("TODO: Finish TeLight::draw");
}

void TeLight::transformDirPoint(const TeVector3f32 &pt1,TeVector3f32 &pt2) {
	const TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), _positionRadial.getX() + M_PI);
	const TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, -1), -_positionRadial.getY());
	pt2.rotate(q2);
	pt2.rotate(q1);
	pt2 += pt1;
}

void TeLight::transformSpotPoint(TeVector3f32 &pt) {
	const TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), _positionRadial.getX());
	const TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, -1), _positionRadial.getY());
	pt.rotate(q2);
	pt.rotate(q1);
	pt += _position3d;
}

void TeLight::update(uint lightno) {
	if (lightno > GL_MAX_LIGHTS)
		error("Invalid light no %d", lightno);
	float col[4];
	const uint glLight = _toGlLight(lightno);
	col[0] = _colAmbient.r() / 255.0f;
	col[1] = _colAmbient.g() / 255.0f;
	col[2] = _colAmbient.b() / 255.0f;
	col[3] = 1.0;
	glLightfv(glLight, GL_AMBIENT, col);

	col[0] = _colDiffuse.r() / 255.0f;
	col[1] = _colDiffuse.g() / 255.0f;
	col[2] = _colDiffuse.b() / 255.0f;
	col[3] = 1.0;
	glLightfv(glLight, GL_DIFFUSE, col);

	// WORKAROUND: Original game sets 0.01 as threshold here to avoid enabling
	// the "shadow" light.  However, CitStation/31130 has shadowlight with values
	// 4,0,0 which means it gets enabled.

	if (col[0] < 0.02f && col[1] < 0.02f && col[2] < 0.02f)
		glDisable(glLight);

	col[0] = _colSpecular.r() / 255.0f;
	col[1] = _colSpecular.g() / 255.0f;
	col[2] = _colSpecular.b() / 255.0f;
	col[3] = 1.0;
	glLightfv(glLight, GL_SPECULAR, col);

	if (_type == LightTypeSpot || _type == LightTypePoint) {
		float pos[4];
		pos[0] = _position3d.x();
		pos[1] = _position3d.y();
		pos[2] = _position3d.z();
		pos[3] = 1.0f;
		glLightfv(glLight, GL_POSITION, pos);
		glLightf(glLight, GL_CONSTANT_ATTENUATION, _constAtten);
		glLightf(glLight, GL_LINEAR_ATTENUATION, _linearAtten);
		glLightf(glLight, GL_QUADRATIC_ATTENUATION, _quadraticAtten);
	}

	if (_type == LightTypeDirectional) {
		float pos[4];
		float cosx = cosf(_positionRadial.getX());
		float cosy = cosf(_positionRadial.getY());
		float sinx = sinf(_positionRadial.getX());
		float siny = sinf(_positionRadial.getY());
		pos[0] = cosx * cosy;
		pos[1] = siny;
		pos[2] = sinx * cosy;
		pos[3] = 0.0;
		glLightfv(glLight, GL_POSITION, pos);
	}

	if (_type == LightTypeSpot) {
		float pos[4];
		float cosx = cosf(_positionRadial.getX());
		float cosy = cosf(_positionRadial.getY());
		float sinx = sinf(_positionRadial.getX());
		float siny = sinf(_positionRadial.getY());
		pos[0] = cosx * cosy;
		pos[1] = siny;
		pos[2] = sinx * cosy;
		pos[3] = 0.0;
		glLightfv(glLight, GL_SPOT_DIRECTION, pos);
		glLightf(glLight, GL_SPOT_CUTOFF, (_cutoff * 180.0) / M_PI);
		glLightf(glLight, GL_SPOT_EXPONENT, _exponent);
	} else {
		glLightf(glLight, GL_SPOT_CUTOFF, 180.0);
	}
}

/*static*/
void TeLight::updateGlobal() {
	float col[4];
	col[0] = _globalAmbientColor.r() / 255.0f;
	col[1] = _globalAmbientColor.g() / 255.0f;
	col[2] = _globalAmbientColor.b() / 255.0f;
	col[3] = 1.0;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

Common::String TeLight::dump() const {
	const char *ltype;
	switch (_type) {
		case LightTypeSpot:
			ltype = "Spot";
			break;
		case LightTypePoint:
			ltype = "Point";
			break;
		case LightTypeDirectional:
			ltype = "Directional";
			break;
		default:
			error("Invalid light type %d", (int)_type);
	}

	return Common::String::format("%sLight(\n\tamb:%s diff:%s spec:%s\n\tpos:%s posRad:%s atten:%.02f %.02f %.02f\n\tcutoff:%.02f exp:%.02f dispSz:%.02f\n)",
		ltype, _colAmbient.dump().c_str(), _colDiffuse.dump().c_str(),
		_colSpecular.dump().c_str(), _position3d.dump().c_str(),
		_positionRadial.dump().c_str(), _constAtten, _linearAtten,
		_quadraticAtten, _cutoff, _exponent, _displaySize);
}


} // end namespace Tetraedge
