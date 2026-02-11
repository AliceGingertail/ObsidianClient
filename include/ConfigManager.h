#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <string>
#include <optional>

namespace obsidian {

class ConfigManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString lastUsername READ lastUsername WRITE setLastUsername NOTIFY lastUsernameChanged)
    Q_PROPERTY(QString currentPeerId READ currentPeerId WRITE setCurrentPeerId NOTIFY currentPeerIdChanged)

public:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager() override = default;

    // Server settings
    QString serverUrl() const;
    void setServerUrl(const QString& url);

    // User settings
    QString lastUsername() const;
    void setLastUsername(const QString& username);

    // Current device
    QString currentPeerId() const;
    void setCurrentPeerId(const QString& peerId);

    // Token storage (secure)
    void saveTokens(const QString& accessToken, const QString& refreshToken);
    std::optional<std::pair<QString, QString>> loadTokens() const;
    void clearTokens();

    // WireGuard config file management
    Q_INVOKABLE bool saveWireGuardConfig(const QString& peerId,
                                          const QString& config,
                                          const QString& privateKey);
    Q_INVOKABLE QString loadWireGuardConfig(const QString& peerId) const;
    Q_INVOKABLE bool deleteWireGuardConfig(const QString& peerId);
    Q_INVOKABLE QStringList listConfigs() const;

    // Config directory
    Q_INVOKABLE static QString configDirectory();
    Q_INVOKABLE static QString configFilePath(const QString& peerId);

signals:
    void serverUrlChanged();
    void lastUsernameChanged();
    void currentPeerIdChanged();

private:
    QSettings m_settings;
};

} // namespace obsidian
