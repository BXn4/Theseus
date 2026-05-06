// title_sanitize.h: clean game/app titles for the dashboard's text
// renderer. The Xbox xfont atlas is mostly Latin-1 / CP1252 plus a
// handful of dingbats; smart quotes, em dashes, ™/®/©, accented
// non-Latin scripts, and emoji all show up as boxes or empty rects.
//
// Title_SanitizeName runs at every import boundary (Title Maker manual
// entry, batch ISO import, Steam library scan, .uixshortcut read) so
// stored names render correctly without users hand-editing them.
//
// The transform is best-effort, not lossless:
//   - Smart quotes (' ' " ") -> ASCII (' " " ")
//   - En/em dashes (- -)     -> "-"
//   - Ellipsis (.)           -> "..."
//   - Bullet (.)             -> "*"
//   - ™ ® © ™              -> dropped
//   - Latin diacritics (e n u ...) -> kept as CP1252 byte
//   - Anything else outside 0x20-0x7E and 0xA0-0xFE -> dropped
//   - Leading/trailing whitespace trimmed
//   - Output capped at outSize - 1 (no length truncation beyond that;
//     callers pick the field width)

#pragma once

#include <stddef.h>

// Sanitize `in` (UTF-8) into `out`. Returns bytes written (excluding NUL).
int Title_SanitizeName(const char* in, char* out, size_t outSize);

// Returns true if Title_SanitizeName(in) would change the string. Useful
// for "Fix Existing Names" UI to count or filter dirty entries before
// committing changes.
bool Title_NeedsSanitize(const char* in);
