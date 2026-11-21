
#include "flint_gfx_common.h"

uint32_t Font::getCount(void) const {
    return count;
}

uint8_t Font::getSpaceWidth(void) const {
    return spaceWidth;
}

uint8_t Font::getStdWidth() const {
    const CharInfo *c = getChar('a');
    return (c != 0) ? c->getWidth() : getSpaceWidth();
}

uint8_t Font::getStdHeight() const {
    return stdHeight;
}

const CharInfo *Font::getChar(uint32_t unicode) const {
    int32_t r = this->count - 1;
    int32_t l = 0;
    while (r >= l) {
        int32_t mid = l + (r - l) / 2;
        const CharInfo *tmp = (CharInfo *)&((uint8_t *)this)[this->vectorTable[mid]];
        if (tmp->unicode == unicode)
            return tmp;
        else if (tmp->unicode > unicode)
            r = mid - 1;
        else
            l = mid + 1;
    }
    return NULL;
}
