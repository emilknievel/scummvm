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

#include "tetraedge/te/te_bezier_curve.h"

namespace Tetraedge {

TeBezierCurve::TeBezierCurve() : _length(0.0), _rawLength(0.0), _lengthNeedsUpdate(true),
_rawLengthNeedsUpdate(true), _numiterations(1000) {
}

//long TeBezierCurve::bounds(int start);

void TeBezierCurve::clear() {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_length = 0.0;
	_controlPoints.clear();
}

void TeBezierCurve::draw() {
	error("TODO: Implement TeBezierCurve::draw");
}

float TeBezierCurve::length() {
	if (_lengthNeedsUpdate) {
		error("TODO: Implement TeBezierCurve::length");
	}
	return _length;
}

float TeBezierCurve::rawLength() {
	if (_rawLengthNeedsUpdate) {
		_rawLengthNeedsUpdate = false;
		_rawLength = 0.0;
		_rawLengths.clear();
		_rawLengths.push_back(0.0);
		if (_controlPoints.size() > 1) {
			for (unsigned int i = 1; i < _controlPoints.size(); i++) {
				const TeVector3f32 diff = _controlPoints[i] - _controlPoints[i - 1];
				float len = diff.length();
				_rawLength += len;
				_rawLengths.push_back(_rawLength);
			}
		}
	}
	return _rawLength;
}

TeVector3f32 TeBezierCurve::retrievePoint(float offset) const {
	error("TODO: Implement TeBezierCurve::retrievePoint");
}

void TeBezierCurve::setControlPoints(const Common::Array<TeVector3f32> &points) {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_controlPoints = points;
}

void TeBezierCurve::setNbIterations(unsigned long iterations) {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_numiterations = iterations;
}


} // end namespace Tetraedge
