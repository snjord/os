
#include "inc/font.h"

Font::Font(pair<char, GLYPH> glyphs[], unsigned int glyphCount,
           GLYPH &charMissingGlyph)
        : m_Missing(charMissingGlyph)
{
    zero(m_CharMap, sizeof(GLYPH *) * CHARMAP_ELEMENTS);

    for (unsigned int x = 0; x < glyphCount; x++)
    {
        m_CharMap[static_cast<unsigned char>(glyphs[x].first)] = &glyphs[x].second;
    }
}

Font::~Font(void)
{
}

GLYPH&
Font::operator[](char const chr)
{
    GLYPH *pGlyph = m_CharMap[static_cast<unsigned char>(chr)];

    if (pGlyph == NULL)
    {
        pGlyph = &m_Missing;
    }

    return *pGlyph;
}

