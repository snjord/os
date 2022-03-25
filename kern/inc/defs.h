
#ifndef DEFINES_H
#define DEFINES_H

#define NULL                0

#define UNREFERENCED(x)     x = x

#define CS_INDEX            0x08
#define DS_INDEX            0x10

typedef unsigned long long size_t;
typedef unsigned long long addr_t;

void  zero(void *addr, size_t bytes);
void  memcpy(void *dst, void *src, size_t bytes);
void* malloc(size_t bytes);
void* aligned_malloc(size_t bytes, unsigned int alignment);
void  mmap(addr_t virt, addr_t phys, size_t bytes);

void  cpu_vendor(char **vendor);
void  cpu_name(char **name);

template <typename T1, typename T2>
struct pair
{
    pair(void) : first(T1()), second(T2())
    { }

    pair(T1 const &first, T2 const &second) : first(first), second(second)
    { }

    T1 first;
    T2 second;
};

template <typename T1, typename T2>
pair<T1, T2> make_pair(T1 first, T2 second)
{
    return pair<T1, T2>(first, second);
}

typedef union color_u
{
    color_u(unsigned int argb) : rgb(argb)
    { }

    color_u(unsigned char red, unsigned char green, unsigned char blue)
        : red(red), green(green),
          blue(blue), alpha(0)
    { }

    color_u(color_u const &clr)
        : rgb(clr.rgb)
    { }
 
    struct
    {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
    };

    unsigned int rgb;
} COLOR;

typedef struct position_s
{
    position_s(unsigned int x, unsigned int y)
        : X(x), Y(y)
    { }

    unsigned int X;
    unsigned int Y;
} POSITION;

typedef struct glyph_s
{
    glyph_s(unsigned char bytesX, unsigned char bytesY, unsigned char *buff)
        : bytesX(bytesX), bytesY(bytesY), buff(buff)
    { }

    unsigned int PixelsX(void) const
    {
        return bytesX * 8;
    }

    unsigned int PixelsY(void) const
    {
        return bytesY;
    }

    unsigned char bytesX;
    unsigned char bytesY;
    unsigned char *buff;
} GLYPH;

#endif

