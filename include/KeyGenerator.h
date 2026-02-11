#pragma once

#include <QObject>
#include <QString>
#include "WireGuardKeys.h"

namespace obsidian {

class KeyGenerator : public QObject {
    Q_OBJECT

public:
    explicit KeyGenerator(QObject* parent = nullptr) : QObject(parent) {}
    ~KeyGenerator() override = default;

    Q_INVOKABLE bool generateKeyPair() {
        auto result = WireGuardKeys::generateKeyPair();
        if (!result) {
            return false;
        }

        m_currentKeyPair = *result;
        m_hasKeys = true;
        return true;
    }

    Q_INVOKABLE QString publicKey() const {
        if (!m_hasKeys) return QString();
        return QString::fromStdString(m_currentKeyPair.publicKeyBase64());
    }

    Q_INVOKABLE QString privateKey() const {
        if (!m_hasKeys) return QString();
        return QString::fromStdString(m_currentKeyPair.privateKeyBase64());
    }

    Q_INVOKABLE bool hasKeys() const {
        return m_hasKeys;
    }

    Q_INVOKABLE void clearKeys() {
        m_currentKeyPair = KeyPair();
        m_hasKeys = false;
    }

private:
    KeyPair m_currentKeyPair;
    bool m_hasKeys = false;
};

} // namespace obsidian
