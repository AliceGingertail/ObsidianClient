#pragma once

#include <string>
#include <array>
#include <optional>
#include <cstdint>

namespace obsidian {

// WireGuard использует Curve25519 для ключей
// Размер ключа: 32 байта
constexpr size_t KEY_SIZE = 32;

struct KeyPair {
    std::array<uint8_t, KEY_SIZE> privateKey;
    std::array<uint8_t, KEY_SIZE> publicKey;

    std::string privateKeyBase64() const;
    std::string publicKeyBase64() const;
};

class WireGuardKeys {
public:
    // Генерация новой ключевой пары
    static std::optional<KeyPair> generateKeyPair();

    // Вычисление публичного ключа из приватного
    static std::array<uint8_t, KEY_SIZE> derivePublicKey(
        const std::array<uint8_t, KEY_SIZE>& privateKey);

    // Конвертация в/из Base64
    static std::string toBase64(const std::array<uint8_t, KEY_SIZE>& key);
    static std::optional<std::array<uint8_t, KEY_SIZE>> fromBase64(const std::string& base64);

private:
    // Clamp private key согласно Curve25519 спецификации
    static void clampPrivateKey(std::array<uint8_t, KEY_SIZE>& key);

    // Curve25519 scalar multiplication
    static void curve25519ScalarMult(
        std::array<uint8_t, KEY_SIZE>& result,
        const std::array<uint8_t, KEY_SIZE>& scalar,
        const std::array<uint8_t, KEY_SIZE>& point);

    // Базовая точка Curve25519 (9)
    static constexpr std::array<uint8_t, KEY_SIZE> BASE_POINT = {9};
};

} // namespace obsidian
