#include "inventory.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "gfx.h"



namespace DM {

Box gBoxFloppyZzzCross = Box(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross
Box gBoxPanel = Box(80, 223, 52, 124); // @ G0032_s_Graphic562_Box_Panel
Box gBoxFood = Box(112, 159, 60, 68); // @ G0035_s_Graphic562_Box_Food
Box gBoxWater = Box(112, 159, 83, 91); // @ G0036_s_Graphic562_Box_Water
Box gBoxPoisoned = Box(112, 207, 105, 119); // @ G0037_s_Graphic562_Box_Poisoned

InventoryMan::InventoryMan(DMEngine *vm) : _vm(vm) {
	_panelContent = kPanelContentFoodWaterPoisoned;
}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	ChampionMan &cm = *_vm->_championMan;
	EventManager &em = *_vm->_eventMan;
	DisplayMan &dm = *_vm->_displayMan;

	if ((championIndex == kChmpionCloseInventory) && !cm._champions[championIndex]._currHealth)
		return;
	if (_vm->_pressingEye || _vm->_pressingMouth)
		return;
	_vm->_stopWaitingForPlayerInput = true;
	int16 invChampOrdinal = _inventoryChampionOrdinal; // copy, as the original will be edited
	if (indexToOrdinal(championIndex) == invChampOrdinal) {
		championIndex = kChmpionCloseInventory;
	}

	Champion *champion;
	if (invChampOrdinal) {
		_inventoryChampionOrdinal = indexToOrdinal(kChampionNone);
		warning("MISSING CODE: F0334_INVENTORY_CloseChest");
		champion = &cm._champions[ordinalToIndex(_inventoryChampionOrdinal)];
		if (champion->_currHealth && !cm._candidateChampionOrdinal) {
			champion->setAttributeFlag(kChampionAttributeStatusBox, true);
			warning("MISSING CODE: F0292_CHAMPION_DrawState");
		}
		if (cm._partyIsSleeping) {
			return;
		}
		if (championIndex == kChmpionCloseInventory) {
			em._refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			em._secondaryMouseInput = gSecondaryMouseInput_Movement;
			warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
			warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
			return;
		}
	}

	dm._useByteBoxCoordinates = false;
	_inventoryChampionOrdinal = indexToOrdinal(championIndex);
	if (!invChampOrdinal) {
		warning("MISSING CODE: F0136_VIDEO_ShadeScreenBox");
	}

	champion = &cm._champions[championIndex];
	int16 w = dm.getWidth(kInventoryGraphicIndice);
	int16 h = dm.getHeight(kInventoryGraphicIndice);
	dm.blitToScreen(dm.getBitmap(kInventoryGraphicIndice), w, 0, 0, 0, w, 0, h, kColorNoTransparency, gDungeonViewport);
	if (cm._candidateChampionOrdinal) {
		dm.clearScreenBox(kColorDarkestGray, gBoxFloppyZzzCross, gDungeonViewport);
	}
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> HEALTH");
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> STAMINA");
	warning("MISSING CODE: F0052_TEXT_PrintToViewport -> MANA");

	warning("MISSING CODE: F0291_CHAMPION_DrawSlot in LOOOOOOOOOOOOP");

	champion->setAttributeFlag(kChampionAttributeViewport, true);
	champion->setAttributeFlag(kChampionAttributeStatusBox, true);
	champion->setAttributeFlag(kChampionAttributePanel, true);
	champion->setAttributeFlag(kChampionAttributeLoad, true);
	champion->setAttributeFlag(kChampionAttributeStatistics, true);
	champion->setAttributeFlag(kChampionAttributeNameTitle, true);

	warning("MISSING CODE: F0292_CHAMPION_DrawState");
	em._mousePointerBitmapUpdated = true;
	em._secondaryMouseInput = gSecondaryMouseInput_ChampionInventory;
	warning("MISSING CODE: set G0444_ps_SecondaryKeyboardInput");
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}

void InventoryMan::drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan._useByteBoxCoordinates = false;
	Box box;
	box._y1 = 0;
	box._y2 = 28 + 1;
	box._x1 = championIndex * kChampionStatusBoxSpacing + 7;
	box._x2 = box._x1 + 31 + 1;
	dispMan.blitToScreen(_vm->_championMan->_champions[championIndex]._portrait, 32, 0, 0, box, kColorNoTransparency);
}

void InventoryMan::drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	Box box;
	box._x1 = x;
	box._x2 = box._x1 + pixelWidth;
	box._y1 = y;
	box._y2 = box._y1 + 6;
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->clearScreenBox(color, box);
}

void InventoryMan::drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
	if (amount < -512) {
		color = kColorRed;
	} else if (amount < 0) {
		color = kColorYellow;
	}

	int16 pixelWidth = amount + 1024;
	if (pixelWidth == 3072) {
		pixelWidth = 3071;
	}
	pixelWidth /= 32;
	drawPanelHorizontalBar(115, y + 2, pixelWidth, kColorBlack);
	drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::drawPanelFoodWaterPoisoned() {
	Champion &champ = _vm->_championMan->_champions[_inventoryChampionOrdinal];
	warning("MISSING CODE: F0334_INVENTORY_CloseChest");
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToScreen(dispMan.getBitmap(kPanelEmptyIndice), 144, 0, 0, gBoxPanel, kColorRed);
	dispMan.blitToScreen(dispMan.getBitmap(kFoodLabelIndice), 48, 0, 0, gBoxFood, kColorDarkestGray);
	dispMan.blitToScreen(dispMan.getBitmap(kWaterLabelIndice), 48, 0, 0, gBoxWater, kColorDarkestGray);
	if (champ._poisonEventCount) {
		dispMan.blitToScreen(dispMan.getBitmap(kPoisionedLabelIndice), 96, 0, 0, gBoxPoisoned, kColorDarkestGray);
	}
	drawPanelFoodOrWaterBar(champ._food, 69, kColorLightBrown);
	drawPanelFoodOrWaterBar(champ._water, 92, kColorBlue);
}

void InventoryMan::drawPanelResurrectReincarnate() {
	_panelContent = kPanelContentResurrectReincarnate;
	_vm->_displayMan->blitToScreen(_vm->_displayMan->getBitmap(kPanelResurectReincaranteIndice), 144, 0, 0, gBoxPanel, kColorDarkGreen, gDungeonViewport);
}

}
