/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/hashmap.h"
#include "common/fs.h"

namespace AGS3 {

/**
 * A support class for scanning a hierarchy of folders for AGS games,
 * and printing output suitable for posting into the detection tables.
 * Scanning starts from the current directory when ScummVM is started
 */
class GameScanner {
	struct Entry {
		Common::String _gameName;
		Common::String _filename;
		Common::String _md5;
		size_t _filesize;
	};
private:
	typedef Common::HashMap<Common::String, Entry> EntryMap;
	EntryMap _games;
private:
	/**
	 * Scan a folder for AGS games
	 */
	void scanFolder(const Common::FSNode &folder);

	/**
	 * Scans a single file that may be an AGS game
	 */
	void scanFile(const Common::String &filename);

	/**
	 * Convert a game name to an appropriate game Id
	 */
	static Common::String convertGameNameToId(const Common::String &name);
public:
	/**
	 * Main execution method
	 */
	void scan();
};

} // namespace AGS3
