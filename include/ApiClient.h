#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <memory>
#include <functional>

namespace obsidian {

struct AuthTokens {
    Q_GADGET
    Q_PROPERTY(QString accessToken MEMBER accessToken)
    Q_PROPERTY(QString refreshToken MEMBER refreshToken)
    Q_PROPERTY(int expiresIn MEMBER expiresIn)
public:
    QString accessToken;
    QString refreshToken;
    int expiresIn = 0;
};

struct PeerInfo {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString deviceName MEMBER deviceName)
    Q_PROPERTY(QString protocol MEMBER protocol)
    Q_PROPERTY(QString ipAddress MEMBER ipAddress)
    Q_PROPERTY(QString publicKey MEMBER publicKey)
    Q_PROPERTY(bool isActive MEMBER isActive)
public:
    QString id;
    QString deviceName;
    QString protocol;
    QString ipAddress;
    QString publicKey;
    bool isActive = false;
};

struct ServerConfig {
    QString endpoint;
    QString serverPublicKey;
    QString address;
    QString dns;
    QString presharedKey;
    QString allowedIPs;
};

class ApiClient : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY authenticationChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit ApiClient(QObject* parent = nullptr);
    ~ApiClient() override = default;

    QString serverUrl() const { return m_serverUrl; }
    void setServerUrl(const QString& url);

    bool isAuthenticated() const { return !m_accessToken.isEmpty(); }
    bool isLoading() const { return m_loading; }

    // Token management
    void setTokens(const QString& accessToken, const QString& refreshToken) {
        m_accessToken = accessToken;
        m_refreshToken = refreshToken;
        emit authenticationChanged();
    }

    // Authentication
    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void registerUser(const QString& username, const QString& email, const QString& password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void refreshToken();

    // Peers
    Q_INVOKABLE void createPeer(const QString& deviceName, const QString& publicKey);
    Q_INVOKABLE void getPeers();
    Q_INVOKABLE void deletePeer(const QString& peerId);
    Q_INVOKABLE void getPeerConfig(const QString& peerId);

signals:
    void serverUrlChanged();
    void authenticationChanged();
    void loadingChanged();

    // Auth signals
    void loginSuccess(const AuthTokens& tokens);
    void loginError(const QString& error);
    void registerSuccess();
    void registerError(const QString& error);
    void tokenRefreshed(const AuthTokens& tokens);

    // Peer signals
    void peerCreated(const PeerInfo& peer, const ServerConfig& config);
    void peerCreateError(const QString& error);
    void peersLoaded(const QList<PeerInfo>& peers);
    void peerDeleted(const QString& peerId);
    void peerConfigLoaded(const QString& peerId, const QString& config);
    void apiError(const QString& error);

private:
    void sendRequest(const QString& endpoint,
                     const QString& method,
                     const QJsonObject& body,
                     std::function<void(const QJsonObject&)> onSuccess,
                     std::function<void(const QString&)> onError);

    void sendArrayRequest(const QString& endpoint,
                          const QString& method,
                          const QJsonObject& body,
                          std::function<void(const QJsonArray&)> onSuccess,
                          std::function<void(const QString&)> onError);

    void setLoading(bool loading);

    QNetworkAccessManager m_networkManager;
    QString m_serverUrl;
    QString m_accessToken;
    QString m_refreshToken;
    bool m_loading = false;
};

} // namespace obsidian

Q_DECLARE_METATYPE(obsidian::PeerInfo)
Q_DECLARE_METATYPE(obsidian::AuthTokens)
Q_DECLARE_METATYPE(obsidian::ServerConfig)
