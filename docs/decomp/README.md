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

## Functions

| Address | Ghidra name | Theseus name | Subsystem | Description |
|---------|-------------|--------------|-----------|-------------|
| 0x00030e70 | `CArrayObject::concat` | `CArrayObject::concat` (xap_vm.cpp) | [VM](VM.md) | Array concatenation builtin invoked from script via `Array.concat(...)`. |
| 0x00031040 | `CDateObject::Construct` | `CDateObject::Construct` (date_node.cpp) | [VM](VM.md) | Date object constructor. Initializes from current system time when called with no args, or parses string/numeric input. |
| 0x00031450 | `CDateObject::getDate` | `CDateObject::getDate` (date_node.cpp) | [VM](VM.md) | Returns the day-of-month component of the bound date. |
| 0x00031510 | `CDateObject::toGMTString` | `CDateObject::toGMTString` (date_node.cpp) | [VM](VM.md) | Formats date as RFC 1123 UTC string. |
| 0x000315c0 | `CDateObject::toLocaleString` | `CDateObject::toLocaleString` (date_node.cpp) | [VM](VM.md) | Formats date using the dashboard's current locale. |
| 0x00031660 | `CDateObject::toUTCString` | `CDateObject::toUTCString` (date_node.cpp) | [VM](VM.md) | Formats date in ISO 8601 UTC form. |
| 0x00031710 | `CDateObject::SetSystemClock` | `CDateObject::SetSystemClock` (date_node.cpp) | [VM](VM.md) | Writes the bound date back to the kernel via `NtSetSystemTime`. |
| 0x00032040 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [VM](VM.md) | String object default constructor. |
| 0x00032090 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [VM](VM.md) | String object constructor from `wchar_t*` literal. |
| 0x00032130 | `CStrObject::CStrObject` | `CStrObject::CStrObject` (xap_vm.cpp) | [VM](VM.md) | String object copy constructor. |
| 0x000321f0 | `CNumObject::ToNum` | `CNumObject::ToNum` (xap_vm.cpp) | [VM](VM.md) | Number-object self coercion. Returns the bound `float` directly without conversion. |
| 0x00032850 | `CStrObject::toLowerCase` | `CStrObject::toLowerCase` (xap_vm.cpp) | [VM](VM.md) | Lowercases the bound UTF-16 string in place using `_wcslwr`. |
| 0x000328c0 | `CStrObject::toUpperCase` | `CStrObject::toUpperCase` (xap_vm.cpp) | [VM](VM.md) | Uppercases the bound UTF-16 string in place using `_wcsupr`. |
| 0x00034d80 | `CFunctionCompiler::ParseExpression` | `CFunctionCompiler::ParseExpression` (xap_compile.cpp) | [VM](VM.md) | Recursive-descent expression parser. Drives the operator-precedence DOPER table to emit operator opcodes. |
| 0x00034ea0 | `CFunctionCompiler::ParseIF` | `CFunctionCompiler::ParseIF` (xap_compile.cpp) | [VM](VM.md) | Compiles `if` / `if/else` to `opCond` + jump fixups. |
| 0x00035170 | `CFunctionCompiler::ParseDO` | `CFunctionCompiler::ParseDO` (xap_compile.cpp) | [VM](VM.md) | Compiles loop constructs (`while`, `do/while`, `for`) to `opCond` + back-edge `opJump`. |
| 0x00035540 | `CFunctionCompiler::ParseBREAK` | `CFunctionCompiler::ParseBREAK` (xap_compile.cpp) | [VM](VM.md) | Emits a forward `opJump` patched to the loop exit by the enclosing parse-loop. |
| 0x00035610 | `CFunctionCompiler::ParseCONTINUE` | `CFunctionCompiler::ParseCONTINUE` (xap_compile.cpp) | [VM](VM.md) | Emits a back-edge `opJump` to the loop condition. |
| 0x00035800 | `CFunctionCompiler::ParseBlock` | `CFunctionCompiler::ParseBlock` (xap_compile.cpp) | [VM](VM.md) | Wraps a brace-delimited statement list in `opFrame` / `opEndFrame` for local scope. |
| 0x00035960 | `CFunctionCompiler::ParseStatement` | `CFunctionCompiler::ParseStatement` (xap_compile.cpp) | [VM](VM.md) | Statement-level dispatcher. Routes by leading token to ParseIF / ParseDO / ParseBlock / etc. Emits `opStatement` line markers. |
| 0x00035ec0 | `CClassCompiler::CompileNode` | `CClassCompiler::CompileNode` (xap_compile.cpp) | [VM](VM.md) | VRML97 node body compiler. Walks `DEF` / `USE` / property assignments and emits scene graph opcodes. |
| 0x00036440 | `CClassCompiler::CompileDot` | `CClassCompiler::CompileDot` (xap_compile.cpp) | [VM](VM.md) | Compiles dotted property access (`a.b.c`). Emits `opDot` chain. |
| 0x00036f70 | `CRunner::FetchString` | `CRunner::FetchString` (xap_vm.cpp) | [VM](VM.md) | Reads an inline UTF-16 string from the bytecode stream. Used by `opStr`, `opVar`, and node-name opcodes. |
| 0x00037510 | `CRunner::Push` | `CRunner::Push` (xap_vm.cpp) | [VM](VM.md) | Pushes a `CObject*` onto the VM evaluation stack. Refcounts the pushed object. |
| 0x00037800 | `CObject::Call` | `CObject::Call` (xap_vm.cpp) | [VM](VM.md) | Method dispatch. Looks up the FND entry by name on the receiver's class, dereferences arguments off the script stack, and invokes the native handler via signature-typed switch (`sig_vv`, `sig_iv`, etc.). |
| 0x00038620 | `CRunner::ExecuteBuiltIn` | `CRunner::ExecuteBuiltIn` (xap_vm.cpp) | [VM](VM.md) | Global builtin dispatcher. Switches by argument-name length and `wcsncmp`s against `EnableInput`, `eval`, `launch`, `alert`, `log`, `DebugBreak`. |
| 0x000390f0 | `CVec3Object::ToStr` | `CVec3Object::ToStr` (xap_vm.cpp) | [VM](VM.md) | Formats a 3-component vector as `"x y z"` for script string coercion. |
| 0x00040320 | `CObject::ToNum` | `CObject::ToNum` (xap_vm.cpp) | [VM](VM.md) | Default numeric coercion for `CObject` subclasses. Subclasses override; base returns 0. |
| 0x00040380 | `CObject::OnSetProperty` | `CObject::OnSetProperty` (xap_vm.cpp) | [VM](VM.md) | Property-write hook. Called after `opAssign` lands a value on a property; subclasses override to react to property changes. |
| 0x0004147c | `CObject::Deref` | `CObject::Deref` (xap_vm.cpp) | [VM](VM.md) | Dereferences a script value. For `CVarObject` resolves to the bound target; for primitive objects returns `this`. |
| 0x00041950 | `CObject::Assign` | `CObject::Assign` (xap_vm.cpp) | [VM](VM.md) | Default assignment handler. Called by `opAssign` when LHS is a script-visible property. |
| 0x00041d80 | `CObject::ToStr` | `CObject::ToStr` (xap_vm.cpp) | [VM](VM.md) | Default string coercion. Returns class name; subclasses override for meaningful output. |
| 0x00041d90 | `CObject::_vtable_default` | `CObject::_vtable_default` (xap_vm.cpp) | [VM](VM.md) | The default vtable instance shared by base `CObject` instances. |

## Coverage status

Seeded with the script VM and compiler. Per subsystem:

| Subsystem | Doc | Coverage |
|-----------|-----|----------|
| Script VM | [VM.md](VM.md) | seed (32 functions, more to come) |
| Scene Graph | [Node.md](Node.md) | not yet started |
| XIP Archives | [XIP.md](XIP.md) | not yet started |
| Animation Nodes | [AnimationNodes.md](AnimationNodes.md) | not yet started |
| Audio System | [AudioSystem.md](AudioSystem.md) | not yet started |
| Camera | [Camera.md](Camera.md) | not yet started |
| Date/Math | [Date.md](Date.md), [Math.md](Math.md) | not yet started |
| File Operations | [FileOps.md](FileOps.md) | not yet started |
| Keyboard | [Keyboard.md](Keyboard.md) | not yet started |
| NtIoSvc | [NtIoSvc.md](NtIoSvc.md) | not yet started |
| Scene Groups | [SceneGroups.md](SceneGroups.md) | not yet started |
| Settings | [Settings.md](Settings.md) | not yet started |
| Shape Render | [ShapeRender.md](ShapeRender.md) | not yet started |
| String Object | [StringObject.md](StringObject.md) | partial via VM |
| Text | [Text.md](Text.md) | not yet started |
| Title Collection | [TitleCollection.md](TitleCollection.md) | not yet started |
| TMAP System | [TmapSystem.md](TmapSystem.md) | not yet started |
| Util | [Util.md](Util.md) | not yet started |
| DVD Playback | (reference-only, no doc yet) | not yet started |
| Original Audio Pump | (reference-only, no doc yet) | not yet started |

Reference-only entries describe binary functions in the dashboard whose Theseus replacement is intentionally minimal or absent (DVD playback was cut, the legacy WMA pump was cut, and so on). They exist in this table for the historical record, not because they ship.
