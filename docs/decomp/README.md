# Decomp Index

This directory holds Theseus's reverse-engineering notes for the 5960 retail Xbox dashboard. The table below is the master cross-reference between the binary and the Theseus reconstruction; every row maps a function in the loaded XBE (by Ghidra label and address) to its corresponding implementation in the Theseus source tree (or marks it as reference-only for subsystems we have not yet implemented). Per-subsystem narrative documents linked in the Coverage Status table at the bottom carry the prose explanations of behavior.

The intent is auditability. Anyone with the same XBE loaded into Ghidra can search by address, compare what we call something against what they would call it, read a one-line description of the behavior, and then click through to the subsystem narrative for the longer story.

## How this table was built

- **Address**: hex offset in the 5960 retail dashboard XBE. Ghidra's default image base is at 0x00010000; addresses below that are XBE headers, addresses above are .text / .rdata / .data per the section table.
- **Ghidra name**: the symbol as labeled in the loaded project. Names were recovered the way binary RE always recovers names: by pulling apart every dashboard binary we could get our hands on and cherry-picking what each one volunteered. The 5960 retail XBE is the target and the last dashboard build Microsoft shipped; earlier revisions in the dashboard's history are the sources we cross-reference against. The principal techniques on the 5960 side are FND/PRD table extraction (the property and function descriptor blocks in the dashboard contain plaintext method names, parsed via custom Ghidra scripts) and XAP script verification (every name reachable from script lands in a known FND row). Roughly half of the binary's 4559 functions are named today.
- **Theseus name**: the corresponding implementation in the Theseus source tree. Empty when the binary function is reference-only. Marked `(reference only)` when we have decompiled the function for documentation but do not build it (for example, the original Microsoft DVD playback chain).
- **Subsystem**: per-area grouping. Links to the narrative doc for that subsystem.
- **Description**: one line, binary analysis only. No source-level commentary.

When in doubt about an entry, run the address through Ghidra and compare. The point of this table is to be checkable.

## Conventions

- Rows are sorted by address ascending (binary memory layout).
- The Ghidra column reflects the live label in the loaded project. If a Ghidra label changes (rename, refinement), that is a row update here.
- Theseus name uses `Class::method` form when the active source tree exposes that name. For free functions, the bare function name. For helpers that were folded into a larger method during reconstruction, the host method.
- Subsystem column links to the relevant narrative doc; if no doc exists yet, the cell is plain text and a doc is on the TODO list.
- XDK library code that is statically linked into the dashboard (D3DX, DirectSound mixer, MCpx audio drivers, AC97, kernel video drivers, CRT helpers) is intentionally outside the scope of this table. Those rows would describe Microsoft library code, not dashboard code.

## Functions

| Address | Ghidra name | Theseus name | Subsystem | Description |
|---------|-------------|--------------|-----------|-------------|
| 0x0002c570 | `CXApp::CXApp` | globals init in `main.cpp` | [App Shell](#app-shell) | Constructor for the original `CXApp` singleton. Theseus replaced this with module-scope globals and free functions; the binary class is preserved for reference. |
| 0x0002d0e0 | `CXApp::Draw` | main loop in `main.cpp` | [App Shell](#app-shell) | Per-frame render entry. Calls scene graph traversal then `Present`. |
| 0x0002d7e0 | `CXApp::InitApp` | `TheseusInit` in `main.cpp` | [App Shell](#app-shell) | Direct3D device creation, scene graph bootstrap, initial XIP load. |
| 0x0002d930 | `CXApp::CleanupApp` | `TheseusShutdown` in `main.cpp` | [App Shell](#app-shell) | Tear-down: scene graph destruction, D3D device release. |
| 0x0002da00 | `XApp` | (vtable / class registration) | [App Shell](#app-shell) | CXApp class-table thunk emitted by the compiler. Theseus's CTheseus equivalent was eliminated; the binary still has it. |
| 0x0002dbc0 | `CXApp::GetStartupClassFile` | startup logic in `main.cpp` | [App Shell](#app-shell) | Resolves the boot XAP file from config. The dashboard's first scene is loaded by string lookup against this path. |
| 0x0002dc50 | `CRecovery::StartRecovery` | `CReboot::Reboot` in `reboot_node.cpp` | [App Shell](#app-shell) | Original entry to factory recovery via `DashRecovery()`. Theseus replaced with `CReboot::Reboot()` returning to firmware; recovery as a concept does not apply on modded hardware. |
| 0x0002def0 | `XAppCreateFile` | `TheseusCreateFile` in `main.cpp` | [App Shell](#app-shell) | Wrapper around `CreateFileA` with dashboard-specific share/disposition defaults. |
| 0x0002ea20 | `CNodeClass::FindByName` | `CNodeClass::FindByName` in `node_system.cpp` | [Scene Graph](Node.md) | Walks the static class registration list (built from `_classCXxxx_CXxxx__2VCNodeClass__A` entries in .data) looking for a UTF-16 class name match. The XAP compiler uses this when binding `DEF` declarations. |
| 0x0002ee60 | `CXAppNode::Advance` | tick loop in `main.cpp` | [Scene Graph](Node.md) | Per-frame tick on the application root node. Drives the time-dependent node graph. |
| 0x00030570 | `CArrayElementReference::Deref` | `CArrayElementReference::Deref` (xap_vm.cpp) | [Script VM](VM.md) | Dereferences an `array[index]` LHS. Allocates the slot if missing for assignment paths. |
| 0x00030e70 | `CArrayObject::concat` | `CArrayObject::concat` (xap_vm.cpp) | [Script VM](VM.md) | Array concatenation builtin invoked from script via `Array.concat(...)`. |
| 0x00031040 | `CDateObject::Construct` | `CDateObject::Construct` (date_node.cpp) | [Script VM](VM.md) | Date object constructor. Initializes from current system time when called with no args, or parses string/numeric input. |
| 0x00031450 | `CDateObject::getDate` | `CDateObject::getDate` (date_node.cpp) | [Script VM](VM.md) | Returns the day-of-month component of the bound date. |
| 0x00031510 | `CDateObject::toGMTString` | `CDateObject::toGMTString` (date_node.cpp) | [Script VM](VM.md) | Formats date as RFC 1123 UTC string. |
| 0x000315c0 | `CDateObject::toLocaleString` | `CDateObject::toLocaleString` (date_node.cpp) | [Script VM](VM.md) | Formats date using the dashboard's current locale. |
| 0x00031660 | `CDateObject::toUTCString` | `CDateObject::toUTCString` (date_node.cpp) | [Script VM](VM.md) | Formats date in ISO 8601 UTC form. |
| 0x00031710 | `CDateObject::SetSystemClock` | `CDateObject::SetSystemClock` (date_node.cpp) | [Script VM](VM.md) | Writes the bound date back to the kernel via `NtSetSystemTime`. |
| 0x00031b50 | `CFolder::Refresh` | `CFolder::Refresh` (file_ops.cpp) | [File Ops](FileOps.md) | Rescans a Folder node's directory and rebuilds its child File / Folder array. Driven by FindFirstFile / FindNextFile. |
| 0x00032040 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [Script VM](VM.md) | String object default constructor. |
| 0x00032090 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [Script VM](VM.md) | String object constructor from `wchar_t*` literal. |
| 0x00032130 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [Script VM](VM.md) | String object copy constructor. |
| 0x000321f0 | `CNumObject::ToNum` | `CNumObject::ToNum` (xap_vm.cpp) | [Script VM](VM.md) | Number-object self coercion. Returns the bound `float` directly without conversion. |
| 0x00032850 | `CStrObject::toLowerCase` | `CStrObject::toLowerCase` (xap_vm.cpp) | [Script VM](VM.md) | Lowercases the bound UTF-16 string in place using `_wcslwr`. |
| 0x000328c0 | `CStrObject::toUpperCase` | `CStrObject::toUpperCase` (xap_vm.cpp) | [Script VM](VM.md) | Uppercases the bound UTF-16 string in place using `_wcsupr`. |
| 0x00034d80 | `CFunctionCompiler::ParseExpression` | `CFunctionCompiler::ParseExpression` (xap_compile.cpp) | [Script VM](VM.md) | Recursive-descent expression parser. Drives the operator-precedence DOPER table to emit operator opcodes. |
| 0x00034ea0 | `CFunctionCompiler::ParseIF` | `CFunctionCompiler::ParseIF` (xap_compile.cpp) | [Script VM](VM.md) | Compiles `if` / `if/else` to `opCond` + jump fixups. |
| 0x00035170 | `CFunctionCompiler::ParseDO` | `CFunctionCompiler::ParseDO` (xap_compile.cpp) | [Script VM](VM.md) | Compiles loop constructs (`while`, `do/while`, `for`) to `opCond` + back-edge `opJump`. |
| 0x00035540 | `CFunctionCompiler::ParseBREAK` | `CFunctionCompiler::ParseBREAK` (xap_compile.cpp) | [Script VM](VM.md) | Emits a forward `opJump` patched to the loop exit by the enclosing parse-loop. |
| 0x00035610 | `CFunctionCompiler::ParseCONTINUE` | `CFunctionCompiler::ParseCONTINUE` (xap_compile.cpp) | [Script VM](VM.md) | Emits a back-edge `opJump` to the loop condition. |
| 0x00035800 | `CFunctionCompiler::ParseBlock` | `CFunctionCompiler::ParseBlock` (xap_compile.cpp) | [Script VM](VM.md) | Wraps a brace-delimited statement list in `opFrame` / `opEndFrame` for local scope. |
| 0x00035960 | `CFunctionCompiler::ParseStatement` | `CFunctionCompiler::ParseStatement` (xap_compile.cpp) | [Script VM](VM.md) | Statement-level dispatcher. Routes by leading token to ParseIF / ParseDO / ParseBlock / etc. Emits `opStatement` line markers. |
| 0x00035ec0 | `CClassCompiler::CompileNode` | `CClassCompiler::CompileNode` (xap_compile.cpp) | [Script VM](VM.md) | VRML97 node body compiler. Walks `DEF` / `USE` / property assignments and emits scene graph opcodes. |
| 0x00036440 | `CClassCompiler::CompileDot` | `CClassCompiler::CompileDot` (xap_compile.cpp) | [Script VM](VM.md) | Compiles dotted property access (`a.b.c`). Emits `opDot` chain. |
| 0x00036f70 | `CRunner::FetchString` | `CRunner::FetchString` (xap_vm.cpp) | [Script VM](VM.md) | Reads an inline UTF-16 string from the bytecode stream. Used by `opStr`, `opVar`, and node-name opcodes. |
| 0x000373a0 | `CLocalVariable::Assign` | `CLocalVariable::Assign` (xap_vm.cpp) | [Script VM](VM.md) | Assignment handler for stack-frame locals. Stores the right-hand value into the local slot and updates refcounts. |
| 0x00037510 | `CRunner::Push` | `CRunner::Push` (xap_vm.cpp) | [Script VM](VM.md) | Pushes a `CObject*` onto the VM evaluation stack. Refcounts the pushed object. |
| 0x00037800 | `CObject::Call` | `CObject::Call` (xap_vm.cpp) | [Script VM](VM.md) | Method dispatch. Looks up the FND entry by name on the receiver's class, dereferences arguments off the script stack, and invokes the native handler via signature-typed switch (`sig_vv`, `sig_iv`, etc.). |
| 0x00038620 | `CRunner::ExecuteBuiltIn` | `CRunner::ExecuteBuiltIn` (xap_vm.cpp) | [Script VM](VM.md) | Global builtin dispatcher. Switches by argument-name length and `wcsncmp`s against `EnableInput`, `eval`, `launch`, `alert`, `log`, `DebugBreak`. |
| 0x000390f0 | `CVec3Object::ToStr` | `CVec3Object::ToStr` (xap_vm.cpp) | [Script VM](VM.md) | Formats a 3-component vector as `"x y z"` for script string coercion. |
| 0x0003a680 | `CConfig::GetAVPackType` | `CConfig::GetAVPackType` (config.cpp) | [Settings](Settings.md) | Reads the AV pack type (composite, S-Video, component, HDMI-via-pack) from the kernel via `XGetAVPack`. |
| 0x0003a740 | `CConfig::GetAVRegion` | `CConfig::GetAVRegion` (config.cpp) | [Settings](Settings.md) | Returns the configured analog video region (NTSC, PAL, etc.). |
| 0x0003a7e0 | `CConfig::GetGameRegion` | `CConfig::GetGameRegion` (config.cpp) | [Settings](Settings.md) | Returns the console's region code (NA / JP / EU). Used to gate region-locked game launches in scripts. |
| 0x0003b2d0 | `CConfig::GetLaunchReason` | `CConfig::GetLaunchReason` (config.cpp) | [Settings](Settings.md) | Reports why the dashboard was booted (cold boot, returned-to-dash, eject, error). |
| 0x0003b860 | `CConfig::GetXdashVersion` | `CConfig::GetXdashVersion` (config.cpp) | [Settings](Settings.md) | Returns the dashboard build version string. |
| 0x0003baf0 | `CRecovery::RecoveryThread` | (reference only) | [App Shell](#app-shell) | Worker thread that performs the original factory-recovery wipe sequence. Theseus does not exercise this on modded hardware. |
| 0x0003bf00 | `CSettingsFile::Save` | `CSettingsFile::Save` (settingsfile.cpp) | [Settings](Settings.md) | Serializes the in-memory settings tree to disk. Driven by the settings panel's "Save" action. |
| 0x0003ca10 | `CTranslator::TranslateStripColon` | `CTranslator::TranslateStripColon` (locale_node.cpp) | [Locale](Locale.md) | Looks up a localized string by key, then strips a trailing `:` if present. Used for menu labels that need both punctuated and bare forms. |
| 0x0003dba0 | `FindObjectInXIP` | `FindObjectInXIP` (xip_archive.cpp) | [XIP Archives](XIP.md) | Free function: searches an opened XIP archive's directory for a named object (texture, mesh, script). |
| 0x0003ddf0 | `CXipFile::Open` | `CXipFile::Open` (xip_archive.cpp) | [XIP Archives](XIP.md) | Opens a `.xip` archive, reads the header, and validates the magic. Returns a `CXipFile*` handle for subsequent lookups. |
| 0x0003dec0 | `CXipFile::StartLoadThread` | `CXipFile::StartLoadThread` (xip_archive.cpp) | [XIP Archives](XIP.md) | Spawns a background loader thread that streams archive payload while the main thread continues. |
| 0x0003e290 | `CXipFile::Find` | `CXipFile::Find` (xip_archive.cpp) | [XIP Archives](XIP.md) | Linear scan of the archive's TOC for a named entry. |
| 0x0003e320 | `CXipFile::FindObject` | `CXipFile::FindObject` (xip_archive.cpp) | [XIP Archives](XIP.md) | Higher-level wrapper around `Find` that also resolves cross-archive references via the global XIP search list. |
| 0x0003f420 | `CGameCopier::CreateDirectoryA` | `CGameCopier::CreateDirectoryA` (file_ops.cpp) | [File Ops](FileOps.md) | Wrapper around the kernel's CreateDirectory with progress-reporting hooks for the copy UI. |
| 0x0003f700 | `CGameCopier::CopyFileA` | `CGameCopier::CopyFileA` (file_ops.cpp) | [File Ops](FileOps.md) | Buffered file copy with progress callback. Used by save-to-MU and game-copy flows. |
| 0x0003f830 | `CGameCopier::DeleteFileA` | `CGameCopier::DeleteFileA` (file_ops.cpp) | [File Ops](FileOps.md) | DeleteFile wrapper. |
| 0x0003f8a0 | `CGameCopier::DeleteDirectory` | `CGameCopier::DeleteDirectory` (file_ops.cpp) | [File Ops](FileOps.md) | Recursive directory removal with progress callback. |
| 0x000401b0 | `CScreenSaver::Advance` | `CScreenSaver::Advance` (settings_nodes.cpp) | [Settings](Settings.md) | Per-frame tick on the screensaver node. Counts down the idle timer and toggles the saver layer when threshold is reached. |
| 0x000402a0 | `CScreenSaver::reset` | `CScreenSaver::reset` (settings_nodes.cpp) | [Settings](Settings.md) | Resets the idle timer to zero. Called on any input event. |
| 0x00040320 | `CObject::ToNum` | `CObject::ToNum` (xap_vm.cpp) | [Script VM](VM.md) | Default numeric coercion for `CObject` subclasses. Subclasses override; base returns 0. |
| 0x00040380 | `CObject::OnSetProperty` | `CObject::OnSetProperty` (xap_vm.cpp) | [Script VM](VM.md) | Property-write hook. Called after `opAssign` lands a value on a property; subclasses override to react to property changes. |
| 0x00040990 | `CTimeDepNode::~CTimeDepNode` | `CTimeDepNode::~CTimeDepNode` (animation_nodes.cpp) | [Animation](AnimationNodes.md) | Destructor for time-dependent nodes (TimeSensor and friends). Unlinks from the time-dep list. |
| 0x0004147c | `CObject::Deref` | `CObject::Deref` (xap_vm.cpp) | [Script VM](VM.md) | Dereferences a script value. For `CVarObject` resolves to the bound target; for primitive objects returns `this`. |
| 0x000417f8 | `CTimeDepNode::OnSetProperty` | `CTimeDepNode::OnSetProperty` (animation_nodes.cpp) | [Animation](AnimationNodes.md) | Reacts to property writes on time-dep nodes. Re-registers the node with the time-dep list when its `enabled` toggles. |
| 0x00041950 | `CObject::Assign` | `CObject::Assign` (xap_vm.cpp) | [Script VM](VM.md) | Default assignment handler. Called by `opAssign` when LHS is a script-visible property. |
| 0x00041c90 | `CTimeSensor::Advance` | `CTimeSensor::Advance` (animation_nodes.cpp) | [Animation](AnimationNodes.md) | Per-frame tick. Computes fractional progress through the sensor's cycle interval and routes that into the bound interpolators. |
| 0x00041d80 | `CObject::ToStr` | `CObject::ToStr` (xap_vm.cpp) | [Script VM](VM.md) | Default string coercion. Returns class name; subclasses override for meaningful output. |
| 0x00041d90 | `CObject::_vtable_default` | `CObject::_vtable_default` (xap_vm.cpp) | [Script VM](VM.md) | The default vtable instance shared by base `CObject` instances. |
| 0x00042740 | `CSavedGameGrid::GetSavedGameCount` | `CSavedGameGrid::GetSavedGameCount` (savegame_grid.cpp) | [Save Games](#save-games) | Returns the total count of saves visible across all attached storage devices, summed from the title array. |
| 0x00042e40 | `CTitleArray::RemoveTitle` | `CTitleArray::RemoveTitle` (title_collection.cpp) | [Title Collection](TitleCollection.md) | Removes a title entry from the global title array (called when a title is uninstalled or the storage holding it is removed). |
| 0x000432f0 | `CFont::Open` | `CFont::Open` (text.cpp) | [Text](Text.md) | Loads an XTF font file from a XIP and constructs the glyph cache. |
| 0x00044240 | `CTitleArray::IsPublisherExists` | `CTitleArray::IsPublisherExists` (title_collection.cpp) | [Title Collection](TitleCollection.md) | Tests whether any title in the array shares a given publisher ID. Used by save-game grouping. |
| 0x00046660 | `CMusicCollection::GetUpdateString` | `CMusicCollection::GetUpdateString` (music_collection.cpp) | [Music](#music) | Builds the localized "Updating soundtracks..." progress string for the music panel. |
| 0x00047970 | `CSavedGameGrid::GetTitleName2` | `CSavedGameGrid::GetTitleName2` (savegame_grid.cpp) | [Save Games](#save-games) | Returns the display name for a title at a given grid index, falling back to the cached `TitleNames.ini` lookup if the live cert is unavailable. |
| 0x00048450 | `CSavedGameGrid::GetUpdateString` | `CSavedGameGrid::GetUpdateString` (savegame_grid.cpp) | [Save Games](#save-games) | Builds the "Updating saved games..." progress string for the memory panel. |
| 0x00048690 | `CSavedGameGrid::RenderIconRow` | `CSavedGameGrid::RenderIconRow` (savegame_grid.cpp) | [Save Games](#save-games) | Renders one row of save tiles. Resolves icons from each title's UDATA `TitleImage.xbx` or DLC's `ContentMeta.xbx`. |
| 0x00049ca0 | `CSavedGameGrid::FormatSavedGameTime` | `CSavedGameGrid::FormatSavedGameTime` (savegame_grid.cpp) | [Save Games](#save-games) | Formats a save's last-modified `FILETIME` into a localized date string for the tile subtitle. |
| 0x0004b1d0 | `CSavedGameGrid::DeleteThread` | `CSavedGameGrid::DeleteThread` (savegame_grid.cpp) | [Save Games](#save-games) | Worker thread that performs the actual save-game delete (file removal + UDATA cleanup) off the UI thread. |
| 0x0004b630 | `CSavedGameGrid::StartDelete` | `CSavedGameGrid::StartDelete` (savegame_grid.cpp) | [Save Games](#save-games) | Kicks off a save-game delete operation. The confirm submenu calls this; UI bug noted in known issues prevents the action from actually firing. |
| 0x0004bf50 | `CMemoryMonitor::UpdateValidDeviceFlags` | `CMemoryMonitor::UpdateValidDeviceFlags` (memory_monitor.cpp) | [Memory Monitor](#memory-monitor) | Polls `XGetDevices` and refreshes the bitmask of attached storage (HDD, MU0, MU1, MU2, MU3). |
| 0x0004c3b0 | `CMemoryMonitor::GetTotalAndFreeBlocks` | `CMemoryMonitor::GetTotalAndFreeBlocks` (memory_monitor.cpp) | [Memory Monitor](#memory-monitor) | Returns total + free blocks on a given device for the bar-chart display. |
| 0x0004c480 | `CMemoryMonitor::Advance` | `CMemoryMonitor::Advance` (memory_monitor.cpp) | [Memory Monitor](#memory-monitor) | Per-frame tick. Triggers re-enumeration on insert / eject events. |
| 0x0004c8c0 | `CMemoryMonitor::FormatDeviceName2` | `CMemoryMonitor::FormatDeviceName2` (memory_monitor.cpp) | [Memory Monitor](#memory-monitor) | Builds a device's display label including the user-set MU name when available. |
| 0x0004ccd0 | `CMemoryMonitor::SetMUName` | `CMemoryMonitor::SetMUName` (memory_monitor.cpp) | [Memory Monitor](#memory-monitor) | Renames a memory unit. Writes the new name to the unit's metadata block via the kernel. |
| 0x0004dbb0 | `CTransform::CalcMatrix` | `CTransform::CalcMatrix` (scene_groups.cpp) | [Scene Groups](SceneGroups.md) | Composes translate / rotate / scale into the node's world matrix. Called when any TRS field changes. |
| 0x0004e0f0 | `CTransform::SetRotation` | `CTransform::SetRotation` (scene_groups.cpp) | [Scene Groups](SceneGroups.md) | Sets the rotation as axis-angle. Marks the matrix dirty for next CalcMatrix. |
| 0x0004e4d0 | `CInline::Init` | `CInline::Init` (scene_groups.cpp) | [Scene Groups](SceneGroups.md) | Inline-node init: opens the referenced XIP / XAP and inserts its root into the local scene graph. |
| 0x0004e740 | `CInline::Advance` | `CInline::Advance` (scene_groups.cpp) | [Scene Groups](SceneGroups.md) | Per-frame tick. If the load thread completed, swaps the placeholder for the loaded sub-tree. |
| 0x00053ed0 | `CFont::LoadGlyph` | `CFont::LoadGlyph` (text.cpp) | [Text](Text.md) | Decompresses a single glyph bitmap from the XTF cache table. |
| 0x0005b630 | `CKeyboard::Backspace` | `CKeyboard::Backspace` (keyboard_node.cpp) | [Keyboard](Keyboard.md) | Handles backspace on the on-screen keyboard. Removes the last character (with surrogate pair awareness). |
| 0x0005d200 | `CLevel::Deactivate` | `CLevel::Deactivate` (scene_groups.cpp) | [Scene Groups](SceneGroups.md) | Marks a Level node inactive. Pauses its time-dep children and skips it during render traversal. |
| 0x0005d300 | `CPositionInterpolator::Interpolate` | `CPositionInterpolator::Interpolate` (animation_nodes.cpp) | [Animation](AnimationNodes.md) | Linear interpolation between key positions based on the bound TimeSensor's fraction. |
| 0x0005d5a0 | `COrientationInterpolator::Interpolate` | `COrientationInterpolator::Interpolate` (animation_nodes.cpp) | [Animation](AnimationNodes.md) | Spherical-linear interpolation between key axis-angle rotations. |
| 0x00061090 | `CBackgroundLoader::Fetch` | `CBackgroundLoader::Fetch` (asset_loader.cpp) | [Asset Loader](AssetLoader.md) | Pulls the next pending request off the background loader queue. The loader thread spins on this. |
| 0x00061760 | `CImageTexture::Load` | `CImageTexture::Load` (asset_loader.cpp) | [Asset Loader](AssetLoader.md) | Loads an XBX (Xbox swizzled) texture from a XIP entry into a D3D texture. |
| 0x00061b40 | `CAudioClip::Cleanup` | `CAudioClip::Cleanup` (audio_system.cpp) | [Audio](AudioSystem.md) | Releases an audio clip's DirectSound buffer and waveform data. |
| 0x00061ba0 | `CAudioClip::Init` | `CAudioClip::Init` (audio_system.cpp) | [Audio](AudioSystem.md) | Loads a clip from a XIP entry, parses the WAV header, allocates a DSound buffer, and uploads the PCM payload. |
| 0x000629a0 | `CAudioBuf::Lock` | `CAudioBuf::Lock` (audio_system.cpp) | [Audio](AudioSystem.md) | DSound `IDirectSoundBuffer::Lock` wrapper. Used by the audio pump to write streamed samples. |
| 0x000629d0 | `CAudioBuf::Unlock` | `CAudioBuf::Unlock` (audio_system.cpp) | [Audio](AudioSystem.md) | DSound `Unlock` wrapper. |
| 0x00062a30 | `CAudioBuf::Stop` | `CAudioBuf::Stop` (audio_system.cpp) | [Audio](AudioSystem.md) | Stops playback of a CAudioBuf and resets its play cursor. |
| 0x00062ad0 | `CAudioBuf::SetFrequency` | `CAudioBuf::SetFrequency` (audio_system.cpp) | [Audio](AudioSystem.md) | Sets the buffer's playback rate. Used for pitch shifts and seek-rate effects. |
| 0x00062fe0 | `CAudioPump::Stop` | `CAudioPump::Stop` (audio_system.cpp) | [Audio](AudioSystem.md) | Stops the audio pump's streaming worker. The dashboard's original WMA pump path; Theseus's MP3 path is `CMP3Pump`. |
| 0x000630d0 | `CAudioPump::FillBuffer` | `CAudioPump::FillBuffer` (audio_system.cpp) | [Audio](AudioSystem.md) | Pump tick. Reads decoded samples from the source decoder and writes them into the playback buffer's free region. |
| 0x000632b0 | `CFilePump::Stop` | `CFilePump::Stop` (audio_system.cpp) | [Audio](AudioSystem.md) | Stops a file-backed audio pump. Closes the source file handle. |
| 0x00063320 | `CFilePump::Initialize` | `CFilePump::Initialize` (audio_system.cpp) | [Audio](AudioSystem.md) | Opens an audio file for streaming, parses its header, and binds a decoder. |
| 0x00063520 | `CWMAPump::Initialize` | (reference only) | [Audio](AudioSystem.md) | Original WMA-streaming pump. Cut from Theseus; replaced with `CMP3Pump` (minimp3-based). Preserved here for binary documentation. |
| 0x000638a0 | `CAudioVisualizer::CalcSpectrum` | `CAudioVisualizer::CalcSpectrum` (audio_system.cpp) | [Audio](AudioSystem.md) | Computes a frequency spectrum from the playback buffer's recent samples for visualizer scenes (FFT-driven dotfields, TMAP modulation). |
| 0x00064ec0 | `CNtIoctlCdromService::DeviceIoControl` | `CNtIoctlCdromService::DeviceIoControl` (ntiosvc.cpp) | [NtIoSvc](NtIoSvc.md) | Routes IOCTLs to the CD-ROM device driver. The dashboard talks to the disc through this; Theseus's reconstruction reuses it for ISO/CCI mount via Hermes. |
| 0x00065150 | `CCDDAStreamer::CCDDAStreamer` | (reference only) | [Audio](AudioSystem.md) | CD digital audio streamer constructor. Used by the original soundtrack rip flow; the rip-CD-to-disk feature is not yet implemented in Theseus. |
| 0x000676c0 | `CDVDPlayer::CheckUserOp` | (reference only) | DVD Playback | Checks whether a given DVD user operation (next chapter, scan, menu) is permitted by the disc's prohibited-operations mask. The full DVD playback chain was cut from Theseus; the desktop build uses libmpv instead. |
| 0x0006e6cb | `CFilePump::GetData` | `CFilePump::GetData` (audio_system.cpp) | [Audio](AudioSystem.md) | Reads the next chunk of compressed data from the file pump's source file. Called by the decoder when its input is empty. |
| 0x0007967f | `CXAppNode::GetTextureSurface` | `CXAppNode::GetTextureSurface` (main.cpp) | [Render](#render) | Returns the underlying D3D `IDirect3DSurface8` for a node's bound texture. Used by render-to-texture nodes. |

## Coverage status

The seed and first expansion cover the application shell, script VM, scene graph backbone, XIP archives, animation, scene groups, file ops, save games, memory monitor, audio core, font / text, keyboard, settings / locale / config, and reference-only entries for cut subsystems (DVD playback, original WMA pump). Per-narrative-doc:

| Subsystem | Doc | Coverage |
|-----------|-----|----------|
| Script VM | [VM.md](VM.md) | 35 functions |
| Scene Graph | [Node.md](Node.md) | 2 functions (more class-table entries pending) |
| XIP Archives | [XIP.md](XIP.md) | 5 functions |
| Animation Nodes | [AnimationNodes.md](AnimationNodes.md) | 4 functions |
| Audio System | [AudioSystem.md](AudioSystem.md) | 12 functions |
| Camera | [Camera.md](Camera.md) | not yet started |
| Date / Math | [Date.md](Date.md), [Math.md](Math.md) | 7 functions (Date) |
| File Operations | [FileOps.md](FileOps.md) | 5 functions |
| Keyboard | [Keyboard.md](Keyboard.md) | 1 function (more pending) |
| Locale | (no doc yet, narrative TODO) | 1 function |
| NtIoSvc | [NtIoSvc.md](NtIoSvc.md) | 1 function |
| Scene Groups | [SceneGroups.md](SceneGroups.md) | 5 functions |
| Settings | [Settings.md](Settings.md) | 7 functions |
| Shape Render | [ShapeRender.md](ShapeRender.md) | not yet started |
| Text | [Text.md](Text.md) | 2 functions |
| Title Collection | [TitleCollection.md](TitleCollection.md) | 2 functions |
| TMAP System | [TmapSystem.md](TmapSystem.md) | not yet started |
| Util | [Util.md](Util.md) | not yet started |
| Save Games | (no doc yet, narrative TODO) | 8 functions |
| Memory Monitor | (no doc yet, narrative TODO) | 5 functions |
| Music Collection | (no doc yet, narrative TODO) | 1 function |
| Render | (no doc yet, narrative TODO) | 1 function |
| App Shell | (no doc yet, narrative TODO) | 9 functions |
| Asset Loader | [AssetLoader.md](AssetLoader.md) (narrative TODO) | 2 functions |
| DVD Playback | (reference-only, no doc) | 1 function |
| Original WMA Pump | (reference-only, folded into AudioSystem.md) | 1 function |
| Original CDDA Streamer | (reference-only, folded into AudioSystem.md) | 1 function |

Reference-only entries describe binary functions in the dashboard whose Theseus replacement is intentionally minimal or absent (DVD playback was cut, the legacy WMA pump was cut, the CDDA streamer was cut). They exist in this table for the historical record, not because they ship.

This page grows as more subsystems get audited. Open issues / corrections welcome.
