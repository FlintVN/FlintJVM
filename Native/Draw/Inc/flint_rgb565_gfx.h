
#include "flint_font.h"
#include "flint_gfx_common.h"

class Rgb565Gfx {
protected:
    const int32_t w;
    const int32_t h;
    const int32_t clipX1;
    const int32_t clipY1;
    const int32_t clipX2;
    const int32_t clipY2;
    uint8_t * const data;

    Rgb565Gfx(int32_t w, int32_t h, int32_t clipX1, int32_t clipY1, int32_t clipX2, int32_t clipY2, uint8_t *data);
public:
    void clear(uint32_t color);
    void drawLine(uint32_t color, uint32_t thickness, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    void drawRect(uint32_t color, uint32_t thickness, int32_t x, int32_t y, int32_t w, int32_t h);
    void fillRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h);
    void fillRoundRect(uint32_t color, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4);
    void fillEllipse(uint32_t color, int32_t x, int32_t y, uint32_t w, uint32_t h);
    void drawLatin1(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y);
    void drawUTF16(uint8_t *str, uint32_t len, Font *font, uint32_t color, int32_t x, int32_t y);
private:
    Rgb565Gfx(const Rgb565Gfx &) = delete;
};
