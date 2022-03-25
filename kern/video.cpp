
#include "inc/video.h"

Video::Video(void)
{
    // Defined address for the current mode's information block
    unsigned char *vbeInfo = reinterpret_cast<unsigned char *>(MODE_BLOCK_ADDR);

    m_fb           = *reinterpret_cast<unsigned int **>(vbeInfo + 0x28);
    m_BytesPerLine = *reinterpret_cast<unsigned short *>(vbeInfo + 0x10);
    m_HorizRes     = *reinterpret_cast<unsigned short *>(vbeInfo + 0x12);
    m_VertRes      = *reinterpret_cast<unsigned short *>(vbeInfo + 0x14);

    m_DwordPerLine = m_BytesPerLine / sizeof(unsigned int);

    // identity map the frame buffer into the virtual address range
    addr_t fbAddr  = reinterpret_cast<addr_t>(m_fb);
    mmap(fbAddr, fbAddr, m_BytesPerLine * m_VertRes);

    // blank the screen
    for (unsigned int y = 0; y < m_VertRes; y++)
    {
        for (unsigned int x = 0; x < m_HorizRes; x++)
        {
            m_fb[y * m_DwordPerLine + x] = 0;
        }
    }

    char *procvendor = NULL;
    cpu_vendor(&procvendor);

    char *procname = NULL;
    cpu_name(&procname);

    if (procvendor != NULL)
    {
        DrawText(POSITION(50, 50), procvendor, Fonts::system, COLOR(0xffffff));
    }

    if (procname != NULL)
    {
        DrawText(POSITION(50, 65), procname, Fonts::system, COLOR(0xffffff));
    }

    DrawText(POSITION(50, 200),
             "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\n"
             "0123456789\n"
             "!.()@",
             Fonts::system,
             COLOR(0xffffff));
}


Video::~Video(void)
{
}

void
Video::DrawGlyph(POSITION pos, GLYPH glyph, COLOR clr)
{
    unsigned char byteX;
    unsigned char bitX;

    for (unsigned int y = 0; y < glyph.PixelsY(); y++)
    {
        for (unsigned int x = 0; x < glyph.PixelsX(); x++)
        {
            byteX = x / 8;
            bitX  = 7 - x % 8;

            if ((glyph.buff[y * glyph.bytesX + byteX] & (1 << bitX)) != 0)
            {
                DrawPixel(POSITION(pos.X + x, pos.Y + y), clr.rgb);
            }
        }
    }

    return;
}

void
Video::DrawText(POSITION pos, char const *text, Font &font, COLOR clr)
{
    POSITION orig = pos;

    while (*text != '\0')
    {
        if (*text == '\n')
        {
            pos.Y += font[' '].PixelsY() + LINE_SPACING_PX;
            pos.X = orig.X;
            text++;

            continue;
        }

        GLYPH &glyph = font[*text];
        DrawGlyph(pos, glyph, clr);

        pos.X += glyph.PixelsX();

        text++;
    }

    return;
}

