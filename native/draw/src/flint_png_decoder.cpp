
#include "miniz.h"
#include "flint_system_api.h"
#include "flint_png_decoder.h"

#define TINFL_FAILED        ((size_t)-1)

static inline __attribute__((always_inline)) uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static inline __attribute__((always_inline)) int32_t paeth(uint8_t a, uint8_t b, uint8_t c) {
    int32_t p = a + b - c;
    int32_t pa = p > a ? p - a : a - p;
    int32_t pb = p > b ? p - b : b - p;
    int32_t pc = p > c ? p - c : c - p;
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

static inline void put565(uint8_t *d, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    d[0] = (uint8_t)(v >> 8);
    d[1] = (uint8_t)v;
}

static void unfilter(uint8_t ft, uint8_t *cur, uint8_t *prev, uint32_t len, uint32_t bpp) {
    switch (ft) {
        case 1: {
            for (uint32_t x = bpp; x < len; x++)
                cur[x] += cur[x - bpp];
            return;
        }
        case 2: {
            if (prev) {
                for (uint32_t x = 0; x < len; x++)
                    cur[x] += prev[x];
            }
            return;
        }
        case 3: {
            if (prev) {
                for (uint32_t x = 0; x < bpp; x++)
                    cur[x] += prev[x] >> 1;
                for (uint32_t x = bpp; x < len; x++)
                    cur[x] += (cur[x - bpp] + prev[x]) >> 1;
            }
            else {
                for (uint32_t x = bpp; x < len; x++)
                    cur[x] += cur[x - bpp] >> 1;
            }
            return;
        }
        case 4: {
            if (prev) {
                for (uint32_t x = 0; x < bpp; x++)
                    cur[x] += paeth(0, prev[x], 0);
                for (uint32_t x = bpp; x < len; x++)
                    cur[x] += paeth(cur[x - bpp], prev[x], prev[x - bpp]);
            }
            else {
                for (uint32_t x = bpp; x < len; x++)
                    cur[x] += paeth(cur[x - bpp], 0, prev[x - bpp]);
            }
            return;
        }
        default:
            return;
    }
}

PngDecoder::PngDecoder(void) {
    bitDepth = 0;
    colorType = 0;
    interlace = 0;
    width = 0;
    height = 0;
    trnsCount = 0;
    idatLen = 0;
    palette = NULL;
    trns = NULL;
    idat = NULL;
}

bool PngDecoder::setImageData(const uint8_t *data, uint32_t len) {
    uint32_t pos = 8;
    while (pos + 12 <= (uint32_t)len) {
        uint32_t clen = be32(data + pos);
        const uint8_t *ctype = data + pos + 4;
        const uint8_t *cdata = data + pos + 8;
        if (pos + 12 + clen > (uint32_t)len) break;
        if (!memcmp(ctype, "IHDR", 4) && clen >= 13) {
            width = be32(cdata);
            height = be32(cdata + 4);
            bitDepth = cdata[8];
            colorType = cdata[9];
            interlace = cdata[12];
        }
        else if (!memcmp(ctype, "PLTE", 4))
            palette = cdata;
        else if (!memcmp(ctype, "tRNS", 4)) {
            trns = cdata;
            trnsCount = clen;
        }
        else if (!memcmp(ctype, "IDAT", 4)) {
            idat = cdata;
            idatLen = clen;
        }
        else if (!memcmp(ctype, "IEND", 4))
            break;
        pos += 12 + clen;
    }
    if (width <= 0 || height <= 0 || width > 2048 || height > 2048 || interlace != 0 || idatLen == 0)
        return false;

    bool subByte = (colorType == 3 && (bitDepth == 1 || bitDepth == 2 || bitDepth == 4));
    if (bitDepth != 8 && bitDepth != 16 && !subByte)
        return false;

    uint32_t channels;
    switch (colorType) {
        case 0: channels = 1; break;   /* gray        */
        case 2: channels = 3; break;   /* rgb         */
        case 3: channels = 1; break;   /* palette     */
        case 4: channels = 2; break;   /* gray+alpha  */
        case 6: channels = 4; break;   /* rgba        */
        default: return false;
    }

    bitsPerPixel = channels * bitDepth;
    rowBytes = (width * bitsPerPixel + 7) / 8;

    return true;
}

int32_t PngDecoder::getWidth(void) const {
    return width;
}

int32_t PngDecoder::getHeight(void) const {
    return height;
}

bool PngDecoder::hasAlpha(void) const {
    return (colorType == 4 || colorType == 6) || ((colorType == 3 || colorType == 0) && trns != NULL);
}

uint8_t *PngDecoder::performDecoding(void) const {
    uint32_t bpp = (bitsPerPixel + 7) / 8;
    if (bpp < 1) bpp = 1;
    uint32_t expanded = (rowBytes + 1) * height;     /* +1 filter byte/row */

    uint8_t *raw = (uint8_t *)FlintAPI::System::malloc(expanded);
    if (!raw) return NULL;
    size_t got = tinfl_decompress_mem_to_mem(raw, expanded, idat, idatLen, TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if (got == TINFL_FAILED || got < expanded) {
        FlintAPI::System::free(raw);
        return NULL;
    }

    uint8_t *prev = NULL;
    for (int y = 0; y < height; y++) {
        uint8_t *cur = raw + y * (rowBytes + 1) + 1;
        unfilter(raw[y * (rowBytes + 1)], cur, prev, rowBytes, bpp);
        prev = cur;
    }

    return raw;
}

void PngDecoder::convertToRgb565(uint8_t *raw, uint8_t *rgb565, uint8_t *alpha) const {
    bool hasAl = hasAlpha();
    uint8_t s = (bitDepth == 16) ? 2 : 1;                  /* sample stride: take the high byte of 16-bit */
    if (hasAl && (colorType == 2))
        memset(alpha, 0xFF, (width * height + 1) >> 1);
    for (uint32_t y = 0; y < height; y++) {
        uint8_t *row = raw + y * (rowBytes + 1) + 1;
        uint8_t *o = &rgb565[(y * width) << 1];

        if (colorType == 2) for (uint32_t x = 0; x < width; x++) {
            const uint8_t *p = row + x * 3 * s;
            put565(&o[x << 1], p[0], p[s], p[2 * s]);
        }
        else if (colorType == 6) for (uint32_t x = 0; x < width; x++) {
            const uint8_t *p = row + x * 4 * s;
            put565(&o[x << 1], p[0], p[s], p[2 * s]);
            if (hasAl) {
                uint8_t al = p[3 * s] >> 4;
                int32_t ai = (y * width + x);
                alpha[ai / 2] = (ai & 1) ? ((alpha[ai / 2] & 0x0F) | (al << 4)) : ((alpha[ai / 2] & 0xF0) | al);
            }
        }
        else if (colorType == 0) for (uint32_t x = 0; x < width; x++) {
            uint8_t gray = row[x * s];
            put565(&o[x << 1], gray, gray, gray);
            if (hasAl) {
                uint8_t al = (trnsCount >= 2 && gray == trns[1]) ? 0 : 0x0F;
                int32_t ai = (y * width + x);
                alpha[ai / 2] = (ai & 1) ? ((alpha[ai / 2] & 0x0F) | (al << 4)) : ((alpha[ai / 2] & 0xF0) | al);
            }
        }
        else if (colorType == 4) for (uint32_t x = 0; x < width; x++) {
            const uint8_t *p = row + x * 2 * s;
            put565(&o[x << 1], p[0], p[0], p[0]);
            if (hasAl) {
                uint8_t al = p[s] >> 4;
                int32_t ai = (y * width + x);
                alpha[ai / 2] = (ai & 1) ? ((alpha[ai / 2] & 0x0F) | (al << 4)) : ((alpha[ai / 2] & 0xF0) | al);
            }
        }
        else for (uint32_t x = 0; x < width; x++) {             /* colorType 3: palette */
            uint32_t idx;
            if (bitDepth == 8)
                idx = row[x];
            else {
                uint8_t perByte = 8 / bitDepth;
                int32_t shift = 8 - bitDepth * ((x % perByte) + 1);
                idx = (row[x / perByte] >> shift) & ((1 << bitDepth) - 1);
            }
            put565(&o[x << 1], palette[idx * 3], palette[idx * 3 + 1], palette[idx * 3 + 2]);
            if (hasAl) {
                uint8_t al = (idx < trnsCount) ? (trns[idx] >> 4) : 0x0F;
                int32_t ai = (y * width + x);
                alpha[ai / 2] = (ai & 1) ? ((alpha[ai / 2] & 0x0F) | (al << 4)) : ((alpha[ai / 2] & 0xF0) | al);
            }
        }
    }
}

bool PngDecoder::decodeToRgb565(uint8_t *rgb565, uint8_t *alpha) const {
    uint8_t *raw = performDecoding();
    if (raw == NULL) return false;
    convertToRgb565(raw, rgb565, alpha);
    FlintAPI::System::free(raw);
    return true;
}
