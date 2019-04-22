#ifndef MULTIFORMATREADER_H
#define MULTIFORMATREADER_H

#include <stdio.h>
#include <stdint.h>

namespace RF {
    class MachineEndianness {
    public:
        typedef enum {
            Little = 0,
            Big
        } EndiannessT;

        MachineEndianness();
        ~MachineEndianness() {}

        int isLittle() {
            return (mFlag == MachineEndianness::Little) ? 1 : 0;
        }

        int isBig() {
            return (mFlag == MachineEndianness::Big) ? 1 : 0;
        }

        EndiannessT Which() {
            return mFlag;
        }

    private:
        EndiannessT mFlag;
    };

    class MultiFormatReader {
        FILE *mpFid;
        MachineEndianness mEnd;
        uint8_t mSwapBuffer[8];

    public:
        typedef enum {
            Int8 = 0,
            Int16,
            Int32,
            Uint8,
            Uint16,
            Uint32,
            Float,
            Double
        } FormatT;

        MultiFormatReader(const char *fileName);
        ~MultiFormatReader();

        void reset();
        size_t readSamples(void *buffer, size_t len,
                           MultiFormatReader::FormatT format,
                           MachineEndianness::EndiannessT end);
        size_t readSamples(void *buffer, size_t len, size_t stride,
                           MultiFormatReader::FormatT format,
                           MachineEndianness::EndiannessT end);
    private:
        size_t read(void *buffer, size_t size, size_t len, size_t stride);
        void swapBytes(void *buffer, size_t size, size_t len);
    };
}

#endif // MULTIFORMATREADER_H
