
#ifndef VIDEO_H
#define VIDEO_H

#include "defs.h"
#include "font.h"

#define MODE_BLOCK_ADDR     0x400200
#define LINE_SPACING_PX     2

class Video
{
public:
    Video();
    virtual ~Video();

    void DrawPixel(POSITION pos, COLOR clr)
    {
        m_fb[pos.Y * m_DwordPerLine + pos.X] = clr.rgb;

        return;
    }

    void DrawGlyph(POSITION pos, GLYPH glyph, COLOR clr);
    void DrawText(POSITION pos, char const *text, Font &font, COLOR clr);

private:
    unsigned int  *m_fb;

    unsigned short m_BytesPerLine;
    unsigned short m_DwordPerLine;

    unsigned short m_HorizRes;
    unsigned short m_VertRes;
};

#endif

