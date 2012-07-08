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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_ftell
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "engines/wintermute/Sys/SysClassRegistry.h"
#include "engines/wintermute/Sys/SysClass.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/SXFile.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXFile, false)

CBScriptable *makeSXFile(CBGame *inGame, CScStack *stack) {
	return new CSXFile(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXFile::CSXFile(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	stack->correctParams(1);
	CScValue *Val = stack->pop();

	_filename = NULL;
	if (!Val->isNULL()) CBUtils::setString(&_filename, Val->getString());

	_readFile = NULL;
	_writeFile = NULL;

	_mode = 0;
	_textMode = false;
}


//////////////////////////////////////////////////////////////////////////
CSXFile::~CSXFile() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXFile::cleanup() {
	delete[] _filename;
	_filename = NULL;
	Close();
}


//////////////////////////////////////////////////////////////////////////
void CSXFile::Close() {
	if (_readFile) {
		Game->_fileManager->closeFile(_readFile);
		_readFile = NULL;
	}
	if ((FILE *)_writeFile) {
		fclose((FILE *)_writeFile);
		_writeFile = NULL;
	}
	_mode = 0;
	_textMode = false;
}

//////////////////////////////////////////////////////////////////////////
const char *CSXFile::scToString() {
	if (_filename) return _filename;
	else return "[file object]";
}

#define FILE_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFilename
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetFilename") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();
		cleanup();
		CBUtils::setString(&_filename, filename);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenAsText / OpenAsBinary
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OpenAsText") == 0 || strcmp(name, "OpenAsBinary") == 0) {
		stack->correctParams(1);
		Close();
		_mode = stack->pop()->getInt(1);
		if (_mode < 1 || _mode > 3) {
			script->runtimeError("File.%s: invalid access mode. Setting read mode.", name);
			_mode = 1;
		}
		if (_mode == 1) {
			_readFile = Game->_fileManager->openFile(_filename);
			if (!_readFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for reading.", Name, _filename);
				Close();
			} else _textMode = strcmp(name, "OpenAsText") == 0;
		} else {
			if (strcmp(name, "OpenAsText") == 0) {
				if (_mode == 2) _writeFile = fopen(_filename, "w+");
				else _writeFile = fopen(_filename, "a+");
			} else {
				if (_mode == 2) _writeFile = fopen(_filename, "wb+");
				else _writeFile = fopen(_filename, "ab+");
			}

			if (!_writeFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for writing.", Name, _filename);
				Close();
			} else _textMode = strcmp(name, "OpenAsText") == 0;
		}

		if (_readFile || _writeFile) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Close") == 0) {
		stack->correctParams(0);
		Close();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPosition") == 0) {
		stack->correctParams(1);
		if (_mode == 0) {
			script->runtimeError("File.%s: File is not open", name);
			stack->pushBool(false);
		} else {
			int Pos = stack->pop()->getInt();
			stack->pushBool(SetPos(Pos));
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delete") == 0) {
		stack->correctParams(0);
		Close();
		stack->pushBool(CBPlatform::DeleteFile(_filename) != false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Copy") == 0) {
		stack->correctParams(2);
		const char *Dest = stack->pop()->getString();
		bool Overwrite = stack->pop()->getBool(true);

		Close();
		stack->pushBool(CBPlatform::CopyFile(_filename, Dest, !Overwrite) != false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadLine") == 0) {
		stack->correctParams(0);
		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;
		bool FoundNewLine = false;
		HRESULT Ret = E_FAIL;
		do {
			Ret = _readFile->read(&b, 1);
			if (Ret != 1) break;

			if (Counter > BufSize) {
				Buf = (byte *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == '\n') {
				Buf[Counter] = '\0';
				FoundNewLine = true;
				break;
			} else if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		} while (SUCCEEDED(Ret));

		if (Counter > BufSize) {
			Buf = (byte *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (!FoundNewLine && Counter == 0) stack->pushNULL();
		else stack->pushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadText") == 0) {
		stack->correctParams(1);
		int TextLen = stack->pop()->getInt();

		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;

		HRESULT Ret = E_FAIL;
		while (Counter < TextLen) {
			Ret = _readFile->read(&b, 1);
			if (Ret != 1) break;

			if (Counter > BufSize) {
				Buf = (byte *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		}

		if (Counter > BufSize) {
			Buf = (byte *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (TextLen > 0 && Counter == 0) stack->pushNULL();
		else stack->pushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteLine / WriteText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteLine") == 0 || strcmp(name, "WriteText") == 0) {
		stack->correctParams(1);
		const char *Line = stack->pop()->getString();
		if (!_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in text mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		if (strcmp(name, "WriteLine") == 0)
			fprintf((FILE *)_writeFile, "%s\n", Line);
		else
			fprintf((FILE *)_writeFile, "%s", Line);

		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ReadBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadBool") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		bool Val;
		if (_readFile->read(&Val, sizeof(bool)) == sizeof(bool)) stack->pushBool(Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadByte") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		byte Val;
		if (_readFile->read(&Val, sizeof(byte)) == sizeof(byte)) stack->pushInt(Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadShort") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		short Val;
		if (_readFile->read(&Val, sizeof(short)) == sizeof(short)) stack->pushInt(65536 + Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadInt / ReadLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadInt") == 0 || strcmp(name, "ReadLong") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		int Val;
		if (_readFile->read(&Val, sizeof(int)) == sizeof(int)) stack->pushInt(Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadFloat") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		float Val;
		if (_readFile->read(&Val, sizeof(float)) == sizeof(float)) stack->pushFloat(Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadDouble") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		double Val;
		if (_readFile->read(&Val, sizeof(double)) == sizeof(double)) stack->pushFloat(Val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadString") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 Size;
		if (_readFile->read(&Size, sizeof(uint32)) == sizeof(uint32)) {
			byte *Str = new byte[Size + 1];
			if (Str) {
				if (_readFile->read(Str, Size) == Size) {
					Str[Size] = '\0';
					stack->pushString((char *)Str);
				}
				delete [] Str;
			} else stack->pushNULL();
		} else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteBool") == 0) {
		stack->correctParams(1);
		bool Val = stack->pop()->getBool();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteByte") == 0) {
		stack->correctParams(1);
		byte Val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteShort") == 0) {
		stack->correctParams(1);
		short Val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteInt / WriteLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteInt") == 0 || strcmp(name, "WriteLong") == 0) {
		stack->correctParams(1);
		int Val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteFloat") == 0) {
		stack->correctParams(1);
		float Val = stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteDouble") == 0) {
		stack->correctParams(1);
		double Val = stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteString") == 0) {
		stack->correctParams(1);
		const char *Val = stack->pop()->getString();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}

		uint32 Size = strlen(Val);
		fwrite(&Size, sizeof(Size), 1, (FILE *)_writeFile);
		fwrite(Val, Size, 1, (FILE *)_writeFile);

		stack->pushBool(true);

		return S_OK;
	}


	else return CBScriptable::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXFile::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("file");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Filename") == 0) {
		_scValue->setString(_filename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Position (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Position") == 0) {
		_scValue->setInt(GetPos());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Length") == 0) {
		_scValue->setInt(GetLength());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextMode") == 0) {
		_scValue->setBool(_textMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccessMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccessMode") == 0) {
		_scValue->setInt(_mode);
		return _scValue;
	}

	else return CBScriptable::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::scSetProperty(const char *name, CScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(name, "Length")==0){
	    int OrigLength = _length;
	    _length = max(value->getInt(0), 0);

	    char PropName[20];
	    if(_length < OrigLength){
	        for(int i=_length; i<OrigLength; i++){
	            sprintf(PropName, "%d", i);
	            _values->DeleteProp(PropName);
	        }
	    }
	    return S_OK;
	}
	else*/ return CBScriptable::scSetProperty(name, value);
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetPos() {
	if (_mode == 1 && _readFile) return _readFile->pos();
	else if ((_mode == 2 || _mode == 3) && _writeFile) return ftell((FILE *)_writeFile);
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSXFile::SetPos(uint32 pos, TSeek origin) {
	if (_mode == 1 && _readFile) return _readFile->seek(pos, origin);
	else if ((_mode == 2 || _mode == 3) && _writeFile) return fseek((FILE *)_writeFile, pos, (int)origin) == 0;
	else return false;
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetLength() {
	if (_mode == 1 && _readFile) return _readFile->size();
	else if ((_mode == 2 || _mode == 3) && _writeFile) {
		uint32 CurrentPos = ftell((FILE *)_writeFile);
		fseek((FILE *)_writeFile, 0, SEEK_END);
		int Ret = ftell((FILE *)_writeFile);
		fseek((FILE *)_writeFile, CurrentPos, SEEK_SET);
		return Ret;
	} else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_mode));
	persistMgr->transfer(TMEMBER(_textMode));

	uint32 Pos = 0;
	if (persistMgr->_saving) {
		Pos = GetPos();
		persistMgr->transfer(TMEMBER(Pos));
	} else {
		persistMgr->transfer(TMEMBER(Pos));

		// try to re-open file if needed
		_writeFile = NULL;
		_readFile = NULL;

		if (_mode != 0) {
			// open for reading
			if (_mode == 1) {
				_readFile = Game->_fileManager->openFile(_filename);
				if (!_readFile) Close();
			}
			// open for writing / appending
			else {
				if (_textMode) {
					if (_mode == 2) _writeFile = fopen(_filename, "w+");
					else _writeFile = fopen(_filename, "a+");
				} else {
					if (_mode == 2) _writeFile = fopen(_filename, "wb+");
					else _writeFile = fopen(_filename, "ab+");
				}
				if (_writeFile) Close();
			}
			SetPos(Pos);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
