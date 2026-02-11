#include "ConfigManager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

namespace obsidian {

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
    , m_settings("ObsidianVPN", "ObsidianClient")
{
    // Ensure config directory exists
    QDir dir(configDirectory());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString ConfigManager::configDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
           + "/wireguard";
}

QString ConfigManager::configFilePath(const QString& peerId) {
    Q_UNUSED(peerId);
    // Use fixed interface name "wg0" because wg-quick requires
    // the filename (without .conf) to be a valid interface name (max 15 chars).
    // UUID names exceed this limit.
    return configDirectory() + "/wg0.conf";
}

QString ConfigManager::serverUrl() const {
    return m_settings.value("server/url", "http://127.0.0.1:8081").toString();
}

void ConfigManager::setServerUrl(const QString& url) {
    if (serverUrl() != url) {
        m_settings.setValue("server/url", url);
        emit serverUrlChanged();
    }
}

QString ConfigManager::lastUsername() const {
    return m_settings.value("user/lastUsername", "").toString();
}

void ConfigManager::setLastUsername(const QString& username) {
    if (lastUsername() != username) {
        m_settings.setValue("user/lastUsername", username);
        emit lastUsernameChanged();
    }
}

QString ConfigManager::currentPeerId() const {
    return m_settings.value("device/currentPeerId", "").toString();
}

void ConfigManager::setCurrentPeerId(const QString& peerId) {
    if (currentPeerId() != peerId) {
        m_settings.setValue("device/currentPeerId", peerId);
        emit currentPeerIdChanged();
    }
}

void ConfigManager::saveTokens(const QString& accessToken, const QString& refreshToken) {
    // В продакшене использовать безопасное хранилище (Keychain/Credential Manager)
    m_settings.setValue("auth/accessToken", accessToken);
    m_settings.setValue("auth/refreshToken", refreshToken);
}

std::optional<std::pair<QString, QString>> ConfigManager::loadTokens() const {
    QString access = m_settings.value("auth/accessToken", "").toString();
    QString refresh = m_settings.value("auth/refreshToken", "").toString();

    if (access.isEmpty() || refresh.isEmpty()) {
        return std::nullopt;
    }

    return std::make_pair(access, refresh);
}

void ConfigManager::clearTokens() {
    m_settings.remove("auth/accessToken");
    m_settings.remove("auth/refreshToken");
}

bool ConfigManager::saveWireGuardConfig(
    const QString& peerId,
    const QString& config,
    const QString& privateKey)
{
    // Ensure directory exists
    QDir dir(configDirectory());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            return false;
        }
    }

    QString filePath = configFilePath(peerId);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Replace placeholder with actual private key
    QString finalConfig = config;
    finalConfig.replace("<ВСТАВЬТЕ_ВАШ_ПРИВАТНЫЙ_КЛЮЧ>", privateKey);

    QTextStream out(&file);
    out << finalConfig;
    file.close();

    // Set restrictive permissions (Unix only)
#ifdef Q_OS_UNIX
    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
#endif

    return true;
}

QString ConfigManager::loadWireGuardConfig(const QString& peerId) const {
    QString filePath = configFilePath(peerId);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream in(&file);
    return in.readAll();
}

bool ConfigManager::deleteWireGuardConfig(const QString& peerId) {
    QString filePath = configFilePath(peerId);
    return QFile::remove(filePath);
}

QStringList ConfigManager::listConfigs() const {
    QDir dir(configDirectory());
    QStringList filters;
    filters << "*.conf";

    QStringList files = dir.entryList(filters, QDir::Files);

    // Remove .conf extension to get peer IDs
    QStringList peerIds;
    for (const QString& file : files) {
        peerIds << file.left(file.length() - 5);
    }

    return peerIds;
}

} // namespace obsidian
