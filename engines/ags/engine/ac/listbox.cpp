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

#include "ags/lib/std/set.h"
#include "ags/engine/ac/listbox.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/gui/guimain.h"
#include "ags/engine/debugging/debug_log.h"

#include "ags/shared/debugging/out.h"
#include "ags/shared/util/path.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/scriptstring.h"
#include "ags/globals.h"
#include "ags/ags.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/config-manager.h"

namespace AGS3 {

using namespace AGS::Shared;




// *** LIST BOX FUNCTIONS

int ListBox_AddItem(GUIListBox *lbb, const char *text) {
	if (lbb->AddItem(text) < 0)
		return 0;

	_G(guis_need_update) = 1;
	return 1;
}

int ListBox_InsertItemAt(GUIListBox *lbb, int index, const char *text) {
	if (lbb->InsertItem(index, text) < 0)
		return 0;

	_G(guis_need_update) = 1;
	return 1;
}

void ListBox_Clear(GUIListBox *listbox) {
	listbox->Clear();
	_G(guis_need_update) = 1;
}

void FillDirList(std::set<String> &files, const String &path) {
	String dirName = Path::GetDirectoryPath(path);
	String filePattern = Path::get_filename(path);

	if (dirName.CompareLeftNoCase(get_install_dir()) == 0) {
		String subDir = dirName.Mid(get_install_dir().GetLength());
		if (!subDir.IsEmpty() && subDir[0u] == '/')
			subDir.ClipLeft(1);
		dirName = ConfMan.get("path");
	} else if (dirName.CompareLeftNoCase(get_save_game_directory()) == 0) {
		// Save files listing
		Common::StringArray matches = g_system->getSavefileManager()->listSavefiles(filePattern);
		for (uint idx = 0; idx < matches.size(); ++idx)
			files.insert(matches[idx]);
		return;

	}

	Common::FSDirectory dir(dirName);
	Common::ArchiveMemberList fileList;
	dir.listMatchingMembers(fileList, filePattern);
	for (Common::ArchiveMemberList::iterator iter = fileList.begin(); iter != fileList.end(); ++iter) {
		files.insert((*iter)->getName());
	}
}

void ListBox_FillDirList(GUIListBox *listbox, const char *filemask) {
	listbox->Clear();
	_G(guis_need_update) = 1;

	ResolvedPath rp;
	if (!ResolveScriptPath(filemask, true, rp))
		return;

	std::set<String> files;
	FillDirList(files, rp.FullPath);
	if (!rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
		FillDirList(files, rp.AltPath);

	for (std::set<String>::const_iterator it = files.begin(); it != files.end(); ++it) {
		listbox->AddItem(*it);
	}
}

int ListBox_GetSaveGameSlots(GUIListBox *listbox, int index) {
	if ((index < 0) || (index >= listbox->ItemCount))
		quit("!ListBox.SaveGameSlot: index out of range");

	return listbox->SavedGameIndex[index];
}

int ListBox_FillSaveGameList(GUIListBox *listbox) {
	listbox->Clear();

	int numsaves = 0;
	long filedates[MAXSAVEGAMES];

	SaveStateList saveList = ::AGS::g_vm->listSaves();

	for (uint idx = 0; idx < saveList.size(); ++idx) {
		if (numsaves >= MAXSAVEGAMES)
			break;

		int saveGameSlot = saveList[idx].getSaveSlot();
		Common::String desc = saveList[idx].getDescription();

		listbox->AddItem(desc);
		listbox->SavedGameIndex[numsaves] = saveGameSlot;
		filedates[numsaves] = 0;
		numsaves++;
	}

	int nn;
	for (nn = 0; nn < numsaves - 1; nn++) {
		for (int kk = 0; kk < numsaves - 1; kk++) { // Date order the games

			if (filedates[kk] < filedates[kk + 1]) { // swap them round
				String tempptr = listbox->Items[kk];
				listbox->Items[kk] = listbox->Items[kk + 1];
				listbox->Items[kk + 1] = tempptr;
				int numtem = listbox->SavedGameIndex[kk];
				listbox->SavedGameIndex[kk] = listbox->SavedGameIndex[kk + 1];
				listbox->SavedGameIndex[kk + 1] = numtem;
				long numted = filedates[kk];
				filedates[kk] = filedates[kk + 1];
				filedates[kk + 1] = numted;
			}
		}
	}

	// update the global savegameindex[] array for backward compatibilty
	for (nn = 0; nn < numsaves; nn++) {
		_GP(play).filenumbers[nn] = listbox->SavedGameIndex[nn];
	}

	_G(guis_need_update) = 1;
	listbox->SetSvgIndex(true);

	if (numsaves >= MAXSAVEGAMES)
		return 1;
	return 0;
}

int ListBox_GetItemAtLocation(GUIListBox *listbox, int x, int y) {

	if (!_GP(guis)[listbox->ParentId].IsDisplayed())
		return -1;

	data_to_game_coords(&x, &y);
	x = (x - listbox->X) - _GP(guis)[listbox->ParentId].X;
	y = (y - listbox->Y) - _GP(guis)[listbox->ParentId].Y;

	if ((x < 0) || (y < 0) || (x >= listbox->Width) || (y >= listbox->Height))
		return -1;

	return listbox->GetItemAt(x, y);
}

char *ListBox_GetItemText(GUIListBox *listbox, int index, char *buffer) {
	if ((index < 0) || (index >= listbox->ItemCount))
		quit("!ListBoxGetItemText: invalid item specified");
	strncpy(buffer, listbox->Items[index], 198);
	buffer[199] = 0;
	return buffer;
}

const char *ListBox_GetItems(GUIListBox *listbox, int index) {
	if ((index < 0) || (index >= listbox->ItemCount))
		quit("!ListBox.Items: invalid index specified");

	return CreateNewScriptString(listbox->Items[index]);
}

void ListBox_SetItemText(GUIListBox *listbox, int index, const char *newtext) {
	if ((index < 0) || (index >= listbox->ItemCount))
		quit("!ListBoxSetItemText: invalid item specified");

	if (strcmp(listbox->Items[index], newtext)) {
		listbox->SetItemText(index, newtext);
		_G(guis_need_update) = 1;
	}
}

void ListBox_RemoveItem(GUIListBox *listbox, int itemIndex) {

	if ((itemIndex < 0) || (itemIndex >= listbox->ItemCount))
		quit("!ListBoxRemove: invalid listindex specified");

	listbox->RemoveItem(itemIndex);
	_G(guis_need_update) = 1;
}

int ListBox_GetItemCount(GUIListBox *listbox) {
	return listbox->ItemCount;
}

int ListBox_GetFont(GUIListBox *listbox) {
	return listbox->Font;
}

void ListBox_SetFont(GUIListBox *listbox, int newfont) {

	if ((newfont < 0) || (newfont >= _GP(game).numfonts))
		quit("!ListBox.Font: invalid font number.");

	if (newfont != listbox->Font) {
		listbox->SetFont(newfont);
		_G(guis_need_update) = 1;
	}

}

bool ListBox_GetShowBorder(GUIListBox *listbox) {
	return listbox->IsBorderShown();
}

void ListBox_SetShowBorder(GUIListBox *listbox, bool newValue) {
	if (listbox->IsBorderShown() != newValue) {
		listbox->SetShowBorder(newValue);
		_G(guis_need_update) = 1;
	}
}

bool ListBox_GetShowScrollArrows(GUIListBox *listbox) {
	return listbox->AreArrowsShown();
}

void ListBox_SetShowScrollArrows(GUIListBox *listbox, bool newValue) {
	if (listbox->AreArrowsShown() != newValue) {
		listbox->SetShowArrows(newValue);
		_G(guis_need_update) = 1;
	}
}

int ListBox_GetHideBorder(GUIListBox *listbox) {
	return !ListBox_GetShowBorder(listbox);
}

void ListBox_SetHideBorder(GUIListBox *listbox, int newValue) {
	ListBox_SetShowBorder(listbox, !newValue);
}

int ListBox_GetHideScrollArrows(GUIListBox *listbox) {
	return !ListBox_GetShowScrollArrows(listbox);
}

void ListBox_SetHideScrollArrows(GUIListBox *listbox, int newValue) {
	ListBox_SetShowScrollArrows(listbox, !newValue);
}

int ListBox_GetSelectedBackColor(GUIListBox *listbox) {
	return listbox->SelectedBgColor;
}

void ListBox_SetSelectedBackColor(GUIListBox *listbox, int colr) {
	if (listbox->SelectedBgColor != colr) {
		listbox->SelectedBgColor = colr;
		_G(guis_need_update) = 1;
	}
}

int ListBox_GetSelectedTextColor(GUIListBox *listbox) {
	return listbox->SelectedTextColor;
}

void ListBox_SetSelectedTextColor(GUIListBox *listbox, int colr) {
	if (listbox->SelectedTextColor != colr) {
		listbox->SelectedTextColor = colr;
		_G(guis_need_update) = 1;
	}
}

int ListBox_GetTextAlignment(GUIListBox *listbox) {
	return listbox->TextAlignment;
}

void ListBox_SetTextAlignment(GUIListBox *listbox, int align) {
	if (listbox->TextAlignment != align) {
		listbox->TextAlignment = (HorAlignment)align;
		_G(guis_need_update) = 1;
	}
}

int ListBox_GetTextColor(GUIListBox *listbox) {
	return listbox->TextColor;
}

void ListBox_SetTextColor(GUIListBox *listbox, int colr) {
	if (listbox->TextColor != colr) {
		listbox->TextColor = colr;
		_G(guis_need_update) = 1;
	}
}

int ListBox_GetSelectedIndex(GUIListBox *listbox) {
	if ((listbox->SelectedItem < 0) || (listbox->SelectedItem >= listbox->ItemCount))
		return -1;
	return listbox->SelectedItem;
}

void ListBox_SetSelectedIndex(GUIListBox *guisl, int newsel) {

	if (newsel >= guisl->ItemCount)
		newsel = -1;

	if (guisl->SelectedItem != newsel) {
		guisl->SelectedItem = newsel;
		if (newsel >= 0) {
			if (newsel < guisl->TopItem)
				guisl->TopItem = newsel;
			if (newsel >= guisl->TopItem + guisl->VisibleItemCount)
				guisl->TopItem = (newsel - guisl->VisibleItemCount) + 1;
		}
		_G(guis_need_update) = 1;
	}

}

int ListBox_GetTopItem(GUIListBox *listbox) {
	return listbox->TopItem;
}

void ListBox_SetTopItem(GUIListBox *guisl, int item) {
	if ((guisl->ItemCount == 0) && (item == 0))
		;  // allow resetting an empty box to the top
	else if ((item >= guisl->ItemCount) || (item < 0))
		quit("!ListBoxSetTopItem: tried to set top to beyond top or bottom of list");

	guisl->TopItem = item;
	_G(guis_need_update) = 1;
}

int ListBox_GetRowCount(GUIListBox *listbox) {
	return listbox->VisibleItemCount;
}

void ListBox_ScrollDown(GUIListBox *listbox) {
	if (listbox->TopItem + listbox->VisibleItemCount < listbox->ItemCount) {
		listbox->TopItem++;
		_G(guis_need_update) = 1;
	}
}

void ListBox_ScrollUp(GUIListBox *listbox) {
	if (listbox->TopItem > 0) {
		listbox->TopItem--;
		_G(guis_need_update) = 1;
	}
}


GUIListBox *is_valid_listbox(int guin, int objn) {
	if ((guin < 0) | (guin >= _GP(game).numgui)) quit("!ListBox: invalid GUI number");
	if ((objn < 0) | (objn >= _GP(guis)[guin].GetControlCount())) quit("!ListBox: invalid object number");
	if (_GP(guis)[guin].GetControlType(objn) != kGUIListBox)
		quit("!ListBox: specified control is not a list box");
	_G(guis_need_update) = 1;
	return (GUIListBox *)_GP(guis)[guin].GetControl(objn);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================



// int (GUIListBox *lbb, const char *text)
RuntimeScriptValue Sc_ListBox_AddItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(GUIListBox, ListBox_AddItem, const char);
}

// void (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_Clear(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIListBox, ListBox_Clear);
}

// void (GUIListBox *listbox, const char *filemask)
RuntimeScriptValue Sc_ListBox_FillDirList(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUIListBox, ListBox_FillDirList, const char);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_FillSaveGameList(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_FillSaveGameList);
}

// int (GUIListBox *listbox, int x, int y)
RuntimeScriptValue Sc_ListBox_GetItemAtLocation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT2(GUIListBox, ListBox_GetItemAtLocation);
}

// char *(GUIListBox *listbox, int index, char *buffer)
RuntimeScriptValue Sc_ListBox_GetItemText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT_POBJ(GUIListBox, char, _GP(myScriptStringImpl), ListBox_GetItemText, char);
}

// int (GUIListBox *lbb, int index, const char *text)
RuntimeScriptValue Sc_ListBox_InsertItemAt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT_POBJ(GUIListBox, ListBox_InsertItemAt, const char);
}

// void (GUIListBox *listbox, int itemIndex)
RuntimeScriptValue Sc_ListBox_RemoveItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_RemoveItem);
}

// void (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_ScrollDown(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIListBox, ListBox_ScrollDown);
}

// void (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_ScrollUp(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIListBox, ListBox_ScrollUp);
}

// void (GUIListBox *listbox, int index, const char *newtext)
RuntimeScriptValue Sc_ListBox_SetItemText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT_POBJ(GUIListBox, ListBox_SetItemText, const char);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetFont);
}

// void (GUIListBox *listbox, int newfont)
RuntimeScriptValue Sc_ListBox_SetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetFont);
}

RuntimeScriptValue Sc_ListBox_GetShowBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(GUIListBox, ListBox_GetShowBorder);
}

RuntimeScriptValue Sc_ListBox_SetShowBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PBOOL(GUIListBox, ListBox_SetShowBorder);
}

RuntimeScriptValue Sc_ListBox_GetShowScrollArrows(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(GUIListBox, ListBox_GetShowScrollArrows);
}

RuntimeScriptValue Sc_ListBox_SetShowScrollArrows(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PBOOL(GUIListBox, ListBox_SetShowScrollArrows);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetHideBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetHideBorder);
}

// void (GUIListBox *listbox, int newValue)
RuntimeScriptValue Sc_ListBox_SetHideBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetHideBorder);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetHideScrollArrows(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetHideScrollArrows);
}

// void (GUIListBox *listbox, int newValue)
RuntimeScriptValue Sc_ListBox_SetHideScrollArrows(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetHideScrollArrows);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetItemCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetItemCount);
}

// const char* (GUIListBox *listbox, int index)
RuntimeScriptValue Sc_ListBox_GetItems(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_PINT(GUIListBox, const char, _GP(myScriptStringImpl), ListBox_GetItems);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetRowCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetRowCount);
}

// int (GUIListBox *listbox, int index)
RuntimeScriptValue Sc_ListBox_GetSaveGameSlots(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(GUIListBox, ListBox_GetSaveGameSlots);
}

RuntimeScriptValue Sc_ListBox_GetSelectedBackColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetSelectedBackColor);
}

// void (GUIListBox *guisl, int newsel)
RuntimeScriptValue Sc_ListBox_SetSelectedBackColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetSelectedBackColor);
}

RuntimeScriptValue Sc_ListBox_GetSelectedTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetSelectedTextColor);
}

// void (GUIListBox *guisl, int newsel)
RuntimeScriptValue Sc_ListBox_SetSelectedTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetSelectedTextColor);
}

RuntimeScriptValue Sc_ListBox_GetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetTextAlignment);
}

// void (GUIListBox *guisl, int newsel)
RuntimeScriptValue Sc_ListBox_SetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetTextAlignment);
}

RuntimeScriptValue Sc_ListBox_GetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetTextColor);
}

// void (GUIListBox *guisl, int newsel)
RuntimeScriptValue Sc_ListBox_SetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetTextColor);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetSelectedIndex(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetSelectedIndex);
}

// void (GUIListBox *guisl, int newsel)
RuntimeScriptValue Sc_ListBox_SetSelectedIndex(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetSelectedIndex);
}

// int (GUIListBox *listbox)
RuntimeScriptValue Sc_ListBox_GetTopItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIListBox, ListBox_GetTopItem);
}

// void (GUIListBox *guisl, int item)
RuntimeScriptValue Sc_ListBox_SetTopItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIListBox, ListBox_SetTopItem);
}



void RegisterListBoxAPI() {
	ccAddExternalObjectFunction("ListBox::AddItem^1", Sc_ListBox_AddItem);
	ccAddExternalObjectFunction("ListBox::Clear^0", Sc_ListBox_Clear);
	ccAddExternalObjectFunction("ListBox::FillDirList^1", Sc_ListBox_FillDirList);
	ccAddExternalObjectFunction("ListBox::FillSaveGameList^0", Sc_ListBox_FillSaveGameList);
	ccAddExternalObjectFunction("ListBox::GetItemAtLocation^2", Sc_ListBox_GetItemAtLocation);
	ccAddExternalObjectFunction("ListBox::GetItemText^2", Sc_ListBox_GetItemText);
	ccAddExternalObjectFunction("ListBox::InsertItemAt^2", Sc_ListBox_InsertItemAt);
	ccAddExternalObjectFunction("ListBox::RemoveItem^1", Sc_ListBox_RemoveItem);
	ccAddExternalObjectFunction("ListBox::ScrollDown^0", Sc_ListBox_ScrollDown);
	ccAddExternalObjectFunction("ListBox::ScrollUp^0", Sc_ListBox_ScrollUp);
	ccAddExternalObjectFunction("ListBox::SetItemText^2", Sc_ListBox_SetItemText);
	ccAddExternalObjectFunction("ListBox::get_Font", Sc_ListBox_GetFont);
	ccAddExternalObjectFunction("ListBox::set_Font", Sc_ListBox_SetFont);
	ccAddExternalObjectFunction("ListBox::get_ShowBorder", Sc_ListBox_GetShowBorder);
	ccAddExternalObjectFunction("ListBox::set_ShowBorder", Sc_ListBox_SetShowBorder);
	ccAddExternalObjectFunction("ListBox::get_ShowScrollArrows", Sc_ListBox_GetShowScrollArrows);
	ccAddExternalObjectFunction("ListBox::set_ShowScrollArrows", Sc_ListBox_SetShowScrollArrows);
	// old "inverted" properties
	ccAddExternalObjectFunction("ListBox::get_HideBorder", Sc_ListBox_GetHideBorder);
	ccAddExternalObjectFunction("ListBox::set_HideBorder", Sc_ListBox_SetHideBorder);
	ccAddExternalObjectFunction("ListBox::get_HideScrollArrows", Sc_ListBox_GetHideScrollArrows);
	ccAddExternalObjectFunction("ListBox::set_HideScrollArrows", Sc_ListBox_SetHideScrollArrows);
	//
	ccAddExternalObjectFunction("ListBox::get_ItemCount", Sc_ListBox_GetItemCount);
	ccAddExternalObjectFunction("ListBox::geti_Items", Sc_ListBox_GetItems);
	ccAddExternalObjectFunction("ListBox::seti_Items", Sc_ListBox_SetItemText);
	ccAddExternalObjectFunction("ListBox::get_RowCount", Sc_ListBox_GetRowCount);
	ccAddExternalObjectFunction("ListBox::geti_SaveGameSlots", Sc_ListBox_GetSaveGameSlots);
	ccAddExternalObjectFunction("ListBox::get_SelectedBackColor", Sc_ListBox_GetSelectedBackColor);
	ccAddExternalObjectFunction("ListBox::set_SelectedBackColor", Sc_ListBox_SetSelectedBackColor);
	ccAddExternalObjectFunction("ListBox::get_SelectedIndex", Sc_ListBox_GetSelectedIndex);
	ccAddExternalObjectFunction("ListBox::set_SelectedIndex", Sc_ListBox_SetSelectedIndex);
	ccAddExternalObjectFunction("ListBox::get_SelectedTextColor", Sc_ListBox_GetSelectedTextColor);
	ccAddExternalObjectFunction("ListBox::set_SelectedTextColor", Sc_ListBox_SetSelectedTextColor);
	ccAddExternalObjectFunction("ListBox::get_TextAlignment", Sc_ListBox_GetTextAlignment);
	ccAddExternalObjectFunction("ListBox::set_TextAlignment", Sc_ListBox_SetTextAlignment);
	ccAddExternalObjectFunction("ListBox::get_TextColor", Sc_ListBox_GetTextColor);
	ccAddExternalObjectFunction("ListBox::set_TextColor", Sc_ListBox_SetTextColor);
	ccAddExternalObjectFunction("ListBox::get_TopItem", Sc_ListBox_GetTopItem);
	ccAddExternalObjectFunction("ListBox::set_TopItem", Sc_ListBox_SetTopItem);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("ListBox::AddItem^1", (void *)ListBox_AddItem);
	ccAddExternalFunctionForPlugin("ListBox::Clear^0", (void *)ListBox_Clear);
	ccAddExternalFunctionForPlugin("ListBox::FillDirList^1", (void *)ListBox_FillDirList);
	ccAddExternalFunctionForPlugin("ListBox::FillSaveGameList^0", (void *)ListBox_FillSaveGameList);
	ccAddExternalFunctionForPlugin("ListBox::GetItemAtLocation^2", (void *)ListBox_GetItemAtLocation);
	ccAddExternalFunctionForPlugin("ListBox::GetItemText^2", (void *)ListBox_GetItemText);
	ccAddExternalFunctionForPlugin("ListBox::InsertItemAt^2", (void *)ListBox_InsertItemAt);
	ccAddExternalFunctionForPlugin("ListBox::RemoveItem^1", (void *)ListBox_RemoveItem);
	ccAddExternalFunctionForPlugin("ListBox::ScrollDown^0", (void *)ListBox_ScrollDown);
	ccAddExternalFunctionForPlugin("ListBox::ScrollUp^0", (void *)ListBox_ScrollUp);
	ccAddExternalFunctionForPlugin("ListBox::SetItemText^2", (void *)ListBox_SetItemText);
	ccAddExternalFunctionForPlugin("ListBox::get_Font", (void *)ListBox_GetFont);
	ccAddExternalFunctionForPlugin("ListBox::set_Font", (void *)ListBox_SetFont);
	ccAddExternalFunctionForPlugin("ListBox::get_HideBorder", (void *)ListBox_GetHideBorder);
	ccAddExternalFunctionForPlugin("ListBox::set_HideBorder", (void *)ListBox_SetHideBorder);
	ccAddExternalFunctionForPlugin("ListBox::get_HideScrollArrows", (void *)ListBox_GetHideScrollArrows);
	ccAddExternalFunctionForPlugin("ListBox::set_HideScrollArrows", (void *)ListBox_SetHideScrollArrows);
	ccAddExternalFunctionForPlugin("ListBox::get_ItemCount", (void *)ListBox_GetItemCount);
	ccAddExternalFunctionForPlugin("ListBox::geti_Items", (void *)ListBox_GetItems);
	ccAddExternalFunctionForPlugin("ListBox::seti_Items", (void *)ListBox_SetItemText);
	ccAddExternalFunctionForPlugin("ListBox::get_RowCount", (void *)ListBox_GetRowCount);
	ccAddExternalFunctionForPlugin("ListBox::geti_SaveGameSlots", (void *)ListBox_GetSaveGameSlots);
	ccAddExternalFunctionForPlugin("ListBox::get_SelectedIndex", (void *)ListBox_GetSelectedIndex);
	ccAddExternalFunctionForPlugin("ListBox::set_SelectedIndex", (void *)ListBox_SetSelectedIndex);
	ccAddExternalFunctionForPlugin("ListBox::get_TopItem", (void *)ListBox_GetTopItem);
	ccAddExternalFunctionForPlugin("ListBox::set_TopItem", (void *)ListBox_SetTopItem);
}

} // namespace AGS3
