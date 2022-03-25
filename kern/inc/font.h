
#ifndef FONT_H
#define FONT_H

#include "defs.h"

#define CHARMAP_ELEMENTS   256

class Font
{
public:
    Font(pair<char, GLYPH> glyphs[], unsigned int glyphCount,
         GLYPH &charMissingGlyph);

    virtual ~Font(void);

    GLYPH& operator[](char const chr);

private:
    GLYPH *m_CharMap[CHARMAP_ELEMENTS];
    GLYPH &m_Missing;

    unsigned int m_Color;
};

namespace Fonts
{
    extern Font system;
}

#endif

