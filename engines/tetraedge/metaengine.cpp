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

#include "tetraedge/metaengine.h"
#include "tetraedge/detection.h"
#include "tetraedge/tetraedge.h"

const char *TetraedgeMetaEngine::getName() const {
	return "tetraedge";
}

Common::Error TetraedgeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tetraedge::TetraedgeEngine(syst, desc);
	return Common::kNoError;
}

bool TetraedgeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(TETRAEDGE)
REGISTER_PLUGIN_DYNAMIC(TETRAEDGE, PLUGIN_TYPE_ENGINE, TetraedgeMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TETRAEDGE, PLUGIN_TYPE_ENGINE, TetraedgeMetaEngine);
#endif
