#pragma once

#include <fstream>
#include <simd/simd.h>
#include "Metal.hpp"

class Hdri {
    public:
        Hdri(MTL::Device *pDevice, const char *fileName);
        ~Hdri();
        MTL::Buffer* getBuffer();
        bool getFlipX();
        bool getFlipY();
        uint32_t getSizeX();
        uint32_t getSizeY();
    private:
        bool _flipX, _flipY;
        uint32_t _sizeX, _sizeY;
        MTL::Buffer *_pDataBuffer;
};