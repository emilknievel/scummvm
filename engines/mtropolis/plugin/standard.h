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

#ifndef MTROPOLIS_PLUGIN_STANDARD_H
#define MTROPOLIS_PLUGIN_STANDARD_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/standard_data.h"

class MidiDriver;

namespace MTropolis {

class Runtime;

namespace Standard {

class StandardPlugIn;
class MidiPlayer;

class CursorModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Cursor Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _applyWhen;
	Event _removeWhen;
	uint32 _cursorID;
};

// Some sort of scene transition modifier
class STransCtModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::STransCtModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Unknown STransCt Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
};

class MediaCueMessengerModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MediaCueMessengerModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Media Cue Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	enum TriggerTiming {
		kTriggerTimingStart = 0,
		kTriggerTimingDuring = 1,
		kTriggerTimingEnd = 2,
	};

	Event _enableWhen;
	Event _disableWhen;
	TriggerTiming _triggerTiming;
	MessengerSendSpec _send;
};

class ObjectReferenceVariableModifier : public VariableModifier {
public:
	ObjectReferenceVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Object Reference Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	MiniscriptInstructionOutcome scriptSetPath(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetObject(MiniscriptThread *thread, const DynamicValue &value);

	void resolve();
	void resolveRelativePath(RuntimeObject *obj, const Common::String &path, size_t startPos);
	void resolveAbsolutePath();

	static bool computeObjectPath(RuntimeObject *obj, Common::String &outPath);
	static RuntimeObject *getObjectParent(RuntimeObject *obj);

	Event _setToSourceParentWhen;

	mutable ObjectReference _object;
	Common::String _objectPath;
	Common::String _fullPath;
};

class MidiModifier : public Modifier {
public:
	MidiModifier();
	~MidiModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MidiModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "MIDI Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	struct FilePart {
		bool loop;
		bool overrideTempo;
		bool volume;
		double tempo;
		double fadeIn;
		double fadeOut;
	};

	struct SingleNotePart {
		uint8 channel;
		uint8 note;
		uint8 velocity;
		uint8 program;
		double duration;
	};

	union ModeSpecificUnion {
		FilePart file;
		SingleNotePart singleNote;
	};

	enum Mode {
		kModeFile,
		kModeSingleNote,
	};

	Event _executeWhen;
	Event _terminateWhen;

	Mode _mode;
	ModeSpecificUnion _modeSpecific;

	Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> _embeddedFile;

	bool _isActive;

	StandardPlugIn *_plugIn;
};

class ListVariableModifier : public VariableModifier {
public:
	ListVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ListVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;


#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "List Variable Modifier"; }
#endif

private:
	ListVariableModifier(const ListVariableModifier &other);
	ListVariableModifier &operator=(const ListVariableModifier &other);

	Common::SharedPtr<Modifier> shallowClone() const override;

	Common::SharedPtr<DynamicList> _list;
};

class SysInfoModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::SysInfoModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "System Info Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
};

struct StandardPlugInHacks {
	StandardPlugInHacks();

	// If list mod values are illegible, just ignore them and flag it as garbled.
	// Necessary to load object 00788ab9 (olL437Check) in Obsidian, which is supposed to be a list of
	// 4 lists that are 3-size each, in the persistent data, but actually contains 4 identical values
	// that appear to be garbage.
	bool allowGarbledListModData;
};

class StandardPlugIn : public MTropolis::PlugIn {
public:
	StandardPlugIn();
	~StandardPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

	const StandardPlugInHacks &getHacks() const;
	StandardPlugInHacks &getHacks();

	MidiPlayer *getMidi() const;

	int8 allocateMidiSource();
	void deallocateMidiSource(int8 source);

private:
	PlugInModifierFactory<CursorModifier, Data::Standard::CursorModifier> _cursorModifierFactory;
	PlugInModifierFactory<STransCtModifier, Data::Standard::STransCtModifier> _sTransCtModifierFactory;
	PlugInModifierFactory<MediaCueMessengerModifier, Data::Standard::MediaCueMessengerModifier> _mediaCueModifierFactory;
	PlugInModifierFactory<ObjectReferenceVariableModifier, Data::Standard::ObjectReferenceVariableModifier> _objRefVarModifierFactory;
	PlugInModifierFactory<MidiModifier, Data::Standard::MidiModifier> _midiModifierFactory;
	PlugInModifierFactory<ListVariableModifier, Data::Standard::ListVariableModifier> _listVarModifierFactory;
	PlugInModifierFactory<SysInfoModifier, Data::Standard::SysInfoModifier> _sysInfoModifierFactory;

	Common::SharedPtr<MidiPlayer> _midi;
	StandardPlugInHacks _hacks;

	Common::Array<int8> _deallocatedSources;
	int8 _lastAllocatedSourceID;
};

} // End of namespace Standard

} // End of namespace MTropolis

#endif
