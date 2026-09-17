#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
namespace Poseidon
{
class Sha256
{
  public:
    void Update(const uint8_t* data, size_t size)
    {
        _bitCount += static_cast<uint64_t>(size) * 8;
        while (size > 0)
        {
            const size_t count = std::min(size, _block.size() - _blockSize);
            std::copy_n(data, count, _block.data() + _blockSize);
            data += count;
            size -= count;
            _blockSize += count;
            if (_blockSize == _block.size())
            {
                Transform(_block.data());
                _blockSize = 0;
            }
        }
    }

    std::array<uint8_t, 32> Finish()
    {
        _block[_blockSize++] = 0x80;
        if (_blockSize > 56)
        {
            std::fill(_block.begin() + _blockSize, _block.end(), 0);
            Transform(_block.data());
            _blockSize = 0;
        }
        std::fill(_block.begin() + _blockSize, _block.begin() + 56, 0);
        for (int i = 0; i < 8; ++i)
            _block[63 - i] = static_cast<uint8_t>(_bitCount >> (i * 8));
        Transform(_block.data());

        std::array<uint8_t, 32> digest{};
        for (size_t i = 0; i < _state.size(); ++i)
            for (int byte = 0; byte < 4; ++byte)
                digest[i * 4 + byte] = static_cast<uint8_t>(_state[i] >> (24 - byte * 8));
        return digest;
    }

  private:
    static uint32_t Rotate(uint32_t value, int count) { return (value >> count) | (value << (32 - count)); }
    void Transform(const uint8_t* block)
    {
        static constexpr uint32_t constants[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        uint32_t words[64];
        for (int i = 0; i < 16; ++i)
            words[i] = (static_cast<uint32_t>(block[i * 4]) << 24) | (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(block[i * 4 + 2]) << 8) | block[i * 4 + 3];
        for (int i = 16; i < 64; ++i)
        {
            const uint32_t s0 = Rotate(words[i - 15], 7) ^ Rotate(words[i - 15], 18) ^ (words[i - 15] >> 3);
            const uint32_t s1 = Rotate(words[i - 2], 17) ^ Rotate(words[i - 2], 19) ^ (words[i - 2] >> 10);
            words[i] = words[i - 16] + s0 + words[i - 7] + s1;
        }
        uint32_t a = _state[0], b = _state[1], c = _state[2], d = _state[3];
        uint32_t e = _state[4], f = _state[5], g = _state[6], h = _state[7];
        for (int i = 0; i < 64; ++i)
        {
            const uint32_t s1 = Rotate(e, 6) ^ Rotate(e, 11) ^ Rotate(e, 25);
            const uint32_t choice = (e & f) ^ (~e & g);
            const uint32_t temp1 = h + s1 + choice + constants[i] + words[i];
            const uint32_t s0 = Rotate(a, 2) ^ Rotate(a, 13) ^ Rotate(a, 22);
            const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = s0 + majority;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
        _state[4] += e;
        _state[5] += f;
        _state[6] += g;
        _state[7] += h;
    }

    std::array<uint32_t, 8> _state = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    std::array<uint8_t, 64> _block{};
    size_t _blockSize = 0;
    uint64_t _bitCount = 0;
};
} // namespace Poseidon
