#include "Hdri.hpp"

Hdri::Hdri(MTL::Device *pDevice, const char *fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) throw;

    char sign, coord;
    std::string line;

    file >> line;
    if (line != "#?RADIANCE") throw;
    
    file >> line;
    if (line != "FORMAT=32-bit_rle_rgbe") throw;
    
    file >> sign >> coord >> this->_sizeY;
    if (coord != 'Y') throw;
    this->_flipY = sign == '-';

    file >> sign >> coord >> this->_sizeX;
    if (coord != 'X') throw;
    this->_flipX = sign == '-';

    unsigned char mode, last, ctr;
    uint32_t *workBuffer = new uint32_t[this->_sizeX * this->_sizeY];
    simd::float4 *textureBuffer = new simd::float4[this->_sizeX * this->_sizeY];
    for (int i = 0; i < this->_sizeY; i++) {
        for (int j = 0; j < this->_sizeX; j++) {
            workBuffer[i * this->_sizeX + j] = 0;
        }
    }
    
    file.get();
    for (int i = 0; i < this->_sizeY; i++) {
        file.get();
        file.get();
        file.get();
        file.get();
        for (int _ = 0; _ < 4; _++) {
            for (int j = 0; j < this->_sizeX; j++) {
                if (!ctr) {
                    ctr = file.get();
                    mode = ctr > 128;
                    last = mode ? file.get() : 0;
                    ctr = mode ? ctr - 128 : ctr;
                }
                ctr--;
                uint32_t data = workBuffer[i * this->_sizeX + j];
                data <<= 8;
                data += mode ? last : file.get();
                workBuffer[i * this->_sizeX + j] = data;
            }
        }
    }
    for (int i = 0; i < this->_sizeY; i++) {
        for (int j = 0; j < this->_sizeX; j++) {
            uint32_t data = workBuffer[(this->_flipY ? this->_sizeY - i - 1 : i) * this->_sizeX + (this->_flipX ? this->_sizeX - j - 1 : j)];
            unsigned char r = (data >> 24) & 0xff;
            unsigned char g = (data >> 16) & 0xff;
            unsigned char b = (data >> 8) & 0xff;
            unsigned char e = data & 0xff;
            float mul = pow(2, e - 128) / 256;
            textureBuffer[i * this->_sizeX + j] = simd::float4{mul * (r + 0.5f), mul * (g + 0.5f), mul * (b + 0.5f), 1};
        }
    }
    this->_pDataBuffer = pDevice->newBuffer(this->_sizeX * this->_sizeY * sizeof(simd::float4), MTL::ResourceStorageModeManaged);
    memcpy(this->_pDataBuffer->contents(), textureBuffer, this->_pDataBuffer->length());
    this->_pDataBuffer->didModifyRange(NS::Range::Make(0, this->_pDataBuffer->length()));
    
    delete[] workBuffer;
    delete[] textureBuffer;
}

Hdri::~Hdri() {
    this->_pDataBuffer->release();
}

MTL::Buffer* Hdri::getBuffer() {
    return this->_pDataBuffer;
}

bool Hdri::getFlipX() {
    return this->_flipX;
}

bool Hdri::getFlipY() {
    return this->_flipY;
}

uint32_t Hdri::getSizeX() {
    return this->_sizeX;
}

uint32_t Hdri::getSizeY() {
    return this->_sizeY;
}