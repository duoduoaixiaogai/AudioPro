#include "multiformatreader.h"

#include <exception>
#include <stdexcept>

namespace Renfeng {
    MachineEndianness::MachineEndianness() {
        mFlag = MachineEndianness::Little;
    }

    MultiFormatReader::MultiFormatReader(const char *fileName)
        : mpFid(nullptr) {
        mpFid = fopen(fileName, "rb");

        if (nullptr == mpFid) {
            throw std::runtime_error("Error opening file");
        }
    }

    MultiFormatReader::~MultiFormatReader() {
        if (nullptr != mpFid) {
            fclose(mpFid);
        }
    }

    void MultiFormatReader::reset() {
        if (nullptr != mpFid) {
            rewind(mpFid);
        }
    }

    size_t MultiFormatReader::readSamples(void *buffer, size_t len,
                                          MultiFormatReader::FormatT format,
                                          MachineEndianness::EndiannessT end) {
        return readSamples(buffer, len, 1, format, end);
    }

    size_t MultiFormatReader::readSamples(void *buffer, size_t len, size_t stride,
                                          MultiFormatReader::FormatT format,
                                          MachineEndianness::EndiannessT end) {
        bool swapflag = (mEnd.Which() != end);
        size_t actRead = 0;

        switch (format) {
            case Int8:
            case Uint8:
                actRead = read(buffer, 1, len, stride);
                break;
            case Int16:
            case Uint16:
                actRead = read(buffer, 2, len, stride);
                if (swapflag) {
                    swapBytes(buffer, 2, len);
                }
                break;
            case Int32:
            case Uint32:
            case Float:
                actRead = read(buffer, 4, len, stride);
                if (swapflag) {
                    swapBytes(buffer, 4, len);
                }
                break;
            case Double:
                actRead = read(buffer, 8, len, stride);
                if (swapflag) {
                    swapBytes(buffer, 8, len);
                }
                break;
            default:
                break;
        }

        return actRead;
    }

    size_t MultiFormatReader::read(void *buffer, size_t size, size_t len, size_t stride) {
        size_t actRead = 0;
        uint8_t *pWork = (uint8_t*)buffer;

        if (stride > 1) {
            for (size_t n = 0; n < len; n++) {
                actRead += fread(&(pWork[n*size]), size, 1, mpFid);
                fseek(mpFid, (stride - 1) * size, SEEK_CUR);
            }
        }
        else {
            actRead = fread(buffer, size, len, mpFid);
        }

        return actRead;
    }

    void MultiFormatReader::swapBytes(void *buffer, size_t size, size_t len) {
        uint8_t *pResBuffer = (uint8_t*)buffer;
        uint8_t *pCurBuffer;

        if (size > 8) {
            throw std::runtime_error("SwapBytes Exception: Format width exceeding 8 bytes.");
        }

        for (size_t i = 0; i < len; i++) {
            pCurBuffer = &(pResBuffer[i * size]);
            memcpy(mSwapBuffer, &(pCurBuffer[0]), size);

            for (size_t n = 0; n < size; n++) {
                pCurBuffer[n] = mSwapBuffer[size - n - 1];
            }
        }
    }
}
