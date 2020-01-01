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

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/files/nuvie_io_file.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima6 {

NuvieIOFileRead::~NuvieIOFileRead() {
	close();
}

bool NuvieIOFileRead::open(const Common::String &filename) {
	if (_file.isOpen()) //We already have a file open lets bail.
		return false;

	if (!_file.open(filename)) {
		DEBUG(0, LEVEL_ERROR, "Failed opening '%s'\n", filename.c_str());
		return false;
	}

	size = _file.size();
	pos = 0;

	return true;
}

void NuvieIOFileRead::close() {
	_file.close();
	NuvieIO::close();
}

void NuvieIOFileRead::seek(uint32 new_pos) {
	if (_file.isOpen() && new_pos <= size) {
		_file.seek(new_pos);
		pos = new_pos;
	}
}

uint8 NuvieIOFileRead::read1() {
	if (pos >= size)
		return 0;

	pos++;

	return _file.readByte();
}

uint16 NuvieIOFileRead::read2() {
	if (pos > size - 2)
		return 0;

	pos += 2;
	return _file.readUint16LE();
}

uint32 NuvieIOFileRead::read4() {
	if (pos > size - 4)
		return 0;

	pos += 4;
	return _file.readUint32LE();
}

bool NuvieIOFileRead::readToBuf(unsigned char *buf, uint32 buf_size) {
	if (pos + buf_size > size)
		return false;

	_file.read(buf, buf_size);
	pos += buf_size;
	return true;
}



// NuvieIOFileWrite
//

NuvieIOFileWrite::NuvieIOFileWrite() : _saveFileData(DisposeAfterUse::YES),
		_file(nullptr), _saveFile(nullptr) {
}

NuvieIOFileWrite::~NuvieIOFileWrite() {
	close();
}

bool NuvieIOFileWrite::open(const Common::String &filename) {
	if (isOpen())
		// We already have an open file
		return false;

	if (filename.contains("/")) {
		// It's a relative path, so we open it using a DumpFile
		if (!_dumpFile.open(filename, true)) {
			DEBUG(0, LEVEL_ERROR, "Failed opening '%s'\n", filename.c_str());
			return false;
		}

		_file = &_dumpFile;
	} else {
		// Singular file, so open it as a save file
		_saveFile = g_system->getSavefileManager()->openForSaving(filename, false);
		assert(_saveFile);

		// Point _file to the _saveFileData member for initial writing,
		// since save files don't allow seeking
		_file = &_saveFileData;
	}

	size = 0;
	pos = 0;

	return true;
}

void NuvieIOFileWrite::close() {
	if (_saveFile) {
		// Writing using savefile interface, so flush out data
		_saveFile->write(_saveFileData.getData(), _saveFileData.size());
		_saveFile->finalize();
		delete _saveFile;
		_saveFile = nullptr;
	} else {
		// Writing to a dump file, so simply close it
		_dumpFile.close();
	}

	_file = nullptr;
}

void NuvieIOFileWrite::seek(uint32 new_pos) {
	if (isOpen() && new_pos <= size) {
		_file->seek(new_pos);
		pos = new_pos;
	}
}

bool NuvieIOFileWrite::write1(uint8 src) {
	if (!isOpen())
		return false;

	_file->writeByte(src);
	++pos;

	if (pos > size)
		size = pos;

	return true;
}

bool NuvieIOFileWrite::write2(uint16 src) {
	if (!isOpen())
		return false;

	_file->writeUint16LE(src);
	pos += 2;

	if (pos > size)
		size = pos;

	return true;
}

bool NuvieIOFileWrite::write4(uint32 src) {
	if (!isOpen())
		return false;

	_file->writeUint32LE(src);
	pos += 4;

	if (pos > size)
		size = pos;

	return true;
}

uint32 NuvieIOFileWrite::writeBuf(const unsigned char *src, uint32 src_size) {
	if (!isOpen())
		return false;

	pos += src_size;
	if (pos > size)
		size = pos;

	return (_file->write(src, src_size));
}

uint32 NuvieIOFileWrite::write(NuvieIO *src) {
	return 0;
}

} // End of namespace Ultima6
} // End of namespace Ultima
