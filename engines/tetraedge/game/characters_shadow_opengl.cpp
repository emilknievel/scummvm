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

#include "graphics/opengl/system_headers.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/characters_shadow_opengl.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_3d_texture_opengl.h"

namespace Tetraedge {

void CharactersShadowOpenGL::createInternal() {
	Te3DTextureOpenGL::unbind();
	glGenTextures(1, &_glTex);
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, _texSize, _texSize, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, nullptr);
}

void CharactersShadowOpenGL::createTextureInternal(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	renderer->clearBuffer(TeRenderer::ColorAndDepth);

	for (Character *character : scene->_characters) {
		character->_model->draw();
	}
	scene->_character->_model->draw();
	Te3DTextureOpenGL::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _texSize, _texSize);
	renderer->clearBuffer(TeRenderer::ColorAndDepth);
}

void CharactersShadowOpenGL::deleteTexture() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->disableTexture();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &_glTex);
}

void CharactersShadowOpenGL::draw(InGameScene *scene) {
	TeRenderer *renderer = g_engine->getRenderer();
	glDepthMask(false);
	renderer->disableZBuffer();
	renderer->enableTexture();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	Te3DTextureOpenGL::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glEnable(GL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	TeMatrix4x4 matrix;
	matrix.translate(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix.scale(TeVector3f32(0.5f, 0.5f, 0.5f));
	matrix = matrix * _camera->projectionMatrix();

	TeMatrix4x4 cammatrix = _camera->worldTransformationMatrix();
	cammatrix.inverse();

	matrix = matrix * cammatrix;

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	float f[4];
	for (uint i = 0; i < 4; i++)
		f[i] = matrix(i, 0);

	glTexGenfv(GL_S, GL_EYE_PLANE, f);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(i, 1);

	glTexGenfv(GL_T, GL_EYE_PLANE, f);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(i, 2);

	glTexGenfv(GL_R, GL_EYE_PLANE, f);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	for (uint i = 0; i < 4; i++)
		f[i] = matrix(i, 3);

	glTexGenfv(GL_Q, GL_EYE_PLANE, f);

	Te3DTextureOpenGL::unbind();
	glBindTexture(GL_TEXTURE_2D, _glTex);
	glEnable(GL_BLEND);
	renderer->setCurrentColor(scene->shadowColor());

	for (TeIntrusivePtr<TeModel> model : scene->zoneModels()) {
		if (model->meshes().size() > 0 && model->meshes()[0]->materials().empty()) {
			model->meshes()[0]->defaultMaterial(TeIntrusivePtr<Te3DTexture>());
			model->meshes()[0]->materials()[0]._enableSomethingDefault0 = true;
			model->meshes()[0]->materials()[0]._diffuseColor = scene->shadowColor();
		}
		model->draw();
	}

	renderer->disableTexture();
	glDepthMask(true);
	renderer->enableZBuffer();
}

} // end namespace Tetraedge
