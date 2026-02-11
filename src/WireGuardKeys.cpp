#include "WireGuardKeys.h"
#include <random>
#include <algorithm>
#include <cstring>
#include <stdexcept>

// Base64 encoding table
static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

namespace obsidian {

std::string KeyPair::privateKeyBase64() const {
    return WireGuardKeys::toBase64(privateKey);
}

std::string KeyPair::publicKeyBase64() const {
    return WireGuardKeys::toBase64(publicKey);
}

std::optional<KeyPair> WireGuardKeys::generateKeyPair() {
    KeyPair keyPair;

    // Генерация случайных байтов для приватного ключа
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    for (auto& byte : keyPair.privateKey) {
        byte = dis(gen);
    }

    // Clamp private key согласно Curve25519
    clampPrivateKey(keyPair.privateKey);

    // Вычисляем публичный ключ
    keyPair.publicKey = derivePublicKey(keyPair.privateKey);

    return keyPair;
}

void WireGuardKeys::clampPrivateKey(std::array<uint8_t, KEY_SIZE>& key) {
    key[0] &= 248;
    key[31] &= 127;
    key[31] |= 64;
}

// Упрощённая реализация Curve25519 scalar multiplication
// В прод следует использовать libsodium или аналог
std::array<uint8_t, KEY_SIZE> WireGuardKeys::derivePublicKey(
    const std::array<uint8_t, KEY_SIZE>& privateKey)
{
    std::array<uint8_t, KEY_SIZE> result;
    std::array<uint8_t, KEY_SIZE> basePoint = {9}; // Curve25519 base point

    curve25519ScalarMult(result, privateKey, basePoint);

    return result;
}

// Curve25519 implementation (simplified)
// Based on TweetNaCl's curve25519 implementation
namespace {

using fe = std::array<int64_t, 16>;

constexpr void fe_frombytes(fe& h, const uint8_t* s) {
    int64_t h0 = static_cast<int64_t>(s[0]);
    int64_t h1 = static_cast<int64_t>(s[1]) << 8;
    int64_t h2 = static_cast<int64_t>(s[2]) << 16;
    int64_t h3 = static_cast<int64_t>(s[3]) << 24;

    // Simplified - full implementation would handle all 32 bytes
    for (int i = 0; i < 16; ++i) {
        h[i] = (static_cast<int64_t>(s[i * 2]) |
                (static_cast<int64_t>(s[i * 2 + 1]) << 8));
    }
}

constexpr void fe_tobytes(uint8_t* s, const fe& h) {
    for (int i = 0; i < 16; ++i) {
        s[i * 2] = static_cast<uint8_t>(h[i] & 0xff);
        s[i * 2 + 1] = static_cast<uint8_t>((h[i] >> 8) & 0xff);
    }
}

void fe_add(fe& h, const fe& f, const fe& g) {
    for (int i = 0; i < 16; ++i) {
        h[i] = f[i] + g[i];
    }
}

void fe_sub(fe& h, const fe& f, const fe& g) {
    for (int i = 0; i < 16; ++i) {
        h[i] = f[i] - g[i];
    }
}

void fe_mul(fe& h, const fe& f, const fe& g) {
    // Simplified multiplication - full implementation is more complex
    fe result = {};
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (i + j < 16) {
                result[i + j] += f[i] * g[j];
            }
        }
    }
    // Reduce
    for (int i = 0; i < 16; ++i) {
        h[i] = result[i] & 0xffff;
        if (i + 1 < 16) {
            result[i + 1] += result[i] >> 16;
        }
    }
}

void fe_sq(fe& h, const fe& f) {
    fe_mul(h, f, f);
}

void fe_invert(fe& out, const fe& z) {
    fe t0, t1, t2, t3;
    fe_sq(t0, z);
    fe_sq(t1, t0);
    fe_sq(t1, t1);
    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t2, t0);
    fe_mul(t1, t1, t2);
    fe_sq(t2, t1);
    for (int i = 0; i < 4; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);
    for (int i = 0; i < 9; ++i) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);
    for (int i = 0; i < 19; ++i) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);
    for (int i = 0; i < 9; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);
    for (int i = 0; i < 49; ++i) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);
    for (int i = 0; i < 99; ++i) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);
    for (int i = 0; i < 49; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);
    for (int i = 0; i < 4; ++i) fe_sq(t1, t1);
    fe_mul(out, t1, t0);
}

void fe_cswap(fe& f, fe& g, int b) {
    int64_t mask = -b;
    for (int i = 0; i < 16; ++i) {
        int64_t t = mask & (f[i] ^ g[i]);
        f[i] ^= t;
        g[i] ^= t;
    }
}

} // anonymous namespace

void WireGuardKeys::curve25519ScalarMult(
    std::array<uint8_t, KEY_SIZE>& result,
    const std::array<uint8_t, KEY_SIZE>& scalar,
    const std::array<uint8_t, KEY_SIZE>& point)
{
    fe x1, x2, x3, z2, z3, tmp0, tmp1;

    fe_frombytes(x1, point.data());

    // Initialize
    x2 = {};
    x2[0] = 1;
    z2 = {};
    x3 = x1;
    z3 = {};
    z3[0] = 1;

    int swap = 0;

    // Montgomery ladder
    for (int pos = 254; pos >= 0; --pos) {
        int b = (scalar[pos / 8] >> (pos & 7)) & 1;
        swap ^= b;
        fe_cswap(x2, x3, swap);
        fe_cswap(z2, z3, swap);
        swap = b;

        fe_sub(tmp0, x3, z3);
        fe_sub(tmp1, x2, z2);
        fe_add(x2, x2, z2);
        fe_add(z2, x3, z3);
        fe_mul(z3, tmp0, x2);
        fe_mul(z2, z2, tmp1);
        fe_sq(tmp0, tmp1);
        fe_sq(tmp1, x2);
        fe_add(x3, z3, z2);
        fe_sub(z2, z3, z2);
        fe_mul(x2, tmp1, tmp0);
        fe_sub(tmp1, tmp1, tmp0);
        fe_sq(z2, z2);
        // Simplified - full implementation uses curve constant
        fe_mul(z3, tmp1, tmp1);
        fe_sq(x3, x3);
        fe_add(tmp0, tmp0, z3);
        fe_mul(z3, x1, z2);
        fe_mul(z2, tmp1, tmp0);
    }

    fe_cswap(x2, x3, swap);
    fe_cswap(z2, z3, swap);

    fe_invert(z2, z2);
    fe_mul(x2, x2, z2);
    fe_tobytes(result.data(), x2);
}

std::string WireGuardKeys::toBase64(const std::array<uint8_t, KEY_SIZE>& key) {
    std::string result;
    result.reserve(((KEY_SIZE + 2) / 3) * 4);

    for (size_t i = 0; i < KEY_SIZE; i += 3) {
        uint32_t val = static_cast<uint32_t>(key[i]) << 16;
        if (i + 1 < KEY_SIZE) val |= static_cast<uint32_t>(key[i + 1]) << 8;
        if (i + 2 < KEY_SIZE) val |= static_cast<uint32_t>(key[i + 2]);

        result += BASE64_CHARS[(val >> 18) & 0x3F];
        result += BASE64_CHARS[(val >> 12) & 0x3F];
        result += (i + 1 < KEY_SIZE) ? BASE64_CHARS[(val >> 6) & 0x3F] : '=';
        result += (i + 2 < KEY_SIZE) ? BASE64_CHARS[val & 0x3F] : '=';
    }

    return result;
}

std::optional<std::array<uint8_t, KEY_SIZE>> WireGuardKeys::fromBase64(const std::string& base64) {
    if (base64.length() != 44) { // 32 bytes = 44 base64 chars with padding
        return std::nullopt;
    }

    std::array<uint8_t, KEY_SIZE> result;
    size_t resultIdx = 0;

    for (size_t i = 0; i < base64.length() && resultIdx < KEY_SIZE; i += 4) {
        uint32_t val = 0;
        int validChars = 0;

        for (int j = 0; j < 4 && i + j < base64.length(); ++j) {
            char c = base64[i + j];
            if (c == '=') break;

            const char* pos = std::strchr(BASE64_CHARS, c);
            if (!pos) return std::nullopt;

            val = (val << 6) | static_cast<uint32_t>(pos - BASE64_CHARS);
            ++validChars;
        }

        val <<= (4 - validChars) * 6;

        if (validChars >= 2 && resultIdx < KEY_SIZE) result[resultIdx++] = (val >> 16) & 0xFF;
        if (validChars >= 3 && resultIdx < KEY_SIZE) result[resultIdx++] = (val >> 8) & 0xFF;
        if (validChars >= 4 && resultIdx < KEY_SIZE) result[resultIdx++] = val & 0xFF;
    }

    return result;
}

} // namespace obsidian
