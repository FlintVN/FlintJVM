
#ifndef __FLINT_PNG_DECODER_H
#define __FLINT_PNG_DECODER_H

class PngDecoder {
private:
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t interlace;
    int32_t width;
    int32_t height;
    int32_t trnsCount;
    uint32_t idatLen;
    uint32_t bitsPerPixel;
    uint32_t rowBytes;
    const uint8_t *palette;
    const uint8_t *trns;
    const uint8_t *idat;

    uint8_t *performDecoding(void) const;
    void convertToRgb565(uint8_t *raw, uint8_t *rgb565, uint8_t *alpha) const;
public:
    PngDecoder(void);

    bool setImageData(const uint8_t *data, uint32_t len);

    int32_t getWidth(void) const;
    int32_t getHeight(void) const;

    bool hasAlpha(void) const;

    bool decodeToRgb565(uint8_t *rgb565, uint8_t *alpha) const;
};

#endif /* __FLINT_PNG_DECODER_H */
