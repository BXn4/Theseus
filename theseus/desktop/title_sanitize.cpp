// title_sanitize.cpp: UTF-8 -> CP1252 best-effort transform for game
// and app titles destined for the dashboard's text renderer. See
// title_sanitize.h for the full rationale + transform table.

#include "title_sanitize.h"

#include <cstring>
#include <cstddef>

namespace {

// Decode one UTF-8 codepoint from `p` (NUL-terminated). Sets `*outCP`
// to the codepoint and returns the number of bytes consumed (1..4).
// On invalid input, consumes one byte and returns U+FFFD so the caller
// can keep advancing without an infinite loop.
static int Utf8Decode(const unsigned char* p, unsigned int* outCP) {
	unsigned char b0 = p[0];
	if (b0 < 0x80) { *outCP = b0; return 1; }
	if ((b0 & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) {
		*outCP = ((b0 & 0x1F) << 6) | (p[1] & 0x3F);
		return 2;
	}
	if ((b0 & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
		*outCP = ((b0 & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
		return 3;
	}
	if ((b0 & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 &&
	    (p[2] & 0xC0) == 0x80 && (p[3] & 0xC0) == 0x80) {
		*outCP = ((b0 & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
		         ((p[2] & 0x3F) << 6)  |  (p[3] & 0x3F);
		return 4;
	}
	*outCP = 0xFFFD;
	return 1;
}

// Map a single codepoint to a CP1252-friendly replacement. Returns the
// number of output bytes written into `out` (0 means "drop this codepoint").
// The 7-bit ASCII and 0xA0-0xFE Latin-1 ranges pass straight through;
// special-case substitutions cover the typographic chars commonly found
// in Steam labels and ISO TitleNames.
static int RemapCodepoint(unsigned int cp, char* out) {
	// Pass-through ranges.
	if (cp >= 0x20 && cp <= 0x7E) { out[0] = (char)cp; return 1; }
	// Latin-1 supplement (e a o n u ... and most Latin diacritics).
	// We drop 0xFF specifically (yuml) -- not in CP1252 reliably.
	if (cp >= 0xA0 && cp <= 0xFE) {
		// Skip ™®© which the dashboard atlas typically lacks.
		if (cp == 0xA9 /* © */ || cp == 0xAE /* ® */) return 0;
		out[0] = (char)cp;
		return 1;
	}

	// Specific typographic substitutions.
	switch (cp) {
		case 0x2018: case 0x2019:           // ' '
			out[0] = '\''; return 1;
		case 0x201C: case 0x201D:           // " "
			out[0] = '"';  return 1;
		case 0x2013: case 0x2014: case 0x2212: // - - -
			out[0] = '-';  return 1;
		case 0x2026:                         // .
			out[0] = '.'; out[1] = '.'; out[2] = '.'; return 3;
		case 0x2022:                         // *
			out[0] = '*';  return 1;
		case 0x00B7:                         // *
			out[0] = '*';  return 1;
		case 0x2122:                         // ™ -- drop
		case 0x00A9:                         // © -- drop
		case 0x00AE:                         // ® -- drop
			return 0;
		case 0x2192: case 0x2190:           // -> <-
			out[0] = (cp == 0x2192) ? '>' : '<'; return 1;
		case 0x00A0:                         // NBSP -> regular space
			out[0] = ' '; return 1;
	}

	// Anything else (CJK, emoji, math symbols, etc.) -- the atlas can't
	// render them, so dropping is better than emitting a tofu box.
	return 0;
}

// Trim leading/trailing whitespace in-place.
static void TrimInPlace(char* s) {
	if (!s || !*s) return;
	char* start = s;
	while (*start == ' ' || *start == '\t') start++;
	if (start != s) memmove(s, start, strlen(start) + 1);
	size_t n = strlen(s);
	while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' ||
	                 s[n - 1] == '\r' || s[n - 1] == '\n')) {
		s[--n] = '\0';
	}
}

// Collapse runs of whitespace into a single space so titles like
// "Halo  :  Combat   Evolved" come out as "Halo : Combat Evolved".
static void CollapseSpaces(char* s) {
	if (!s) return;
	char* w = s;
	bool prevSpace = false;
	for (char* r = s; *r; r++) {
		if (*r == ' ' || *r == '\t') {
			if (!prevSpace) { *w++ = ' '; prevSpace = true; }
		} else {
			*w++ = *r;
			prevSpace = false;
		}
	}
	*w = '\0';
}

} // namespace

int Title_SanitizeName(const char* in, char* out, size_t outSize) {
	if (!in || !out || outSize == 0) return 0;
	const unsigned char* p = (const unsigned char*)in;
	size_t pos = 0;
	while (*p && pos + 4 < outSize) {
		unsigned int cp = 0;
		int consumed = Utf8Decode(p, &cp);
		char buf[4];
		int n = RemapCodepoint(cp, buf);
		for (int i = 0; i < n && pos + 1 < outSize; i++) out[pos++] = buf[i];
		p += consumed;
	}
	out[pos] = '\0';
	TrimInPlace(out);
	CollapseSpaces(out);
	return (int)strlen(out);
}

bool Title_NeedsSanitize(const char* in) {
	if (!in) return false;
	char buf[256];
	Title_SanitizeName(in, buf, sizeof(buf));
	return strcmp(in, buf) != 0;
}
