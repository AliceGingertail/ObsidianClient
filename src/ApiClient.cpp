#include "ApiClient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>

namespace obsidian {

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
{
}

void ApiClient::setServerUrl(const QString& url) {
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
    }
}

void ApiClient::setLoading(bool loading) {
    if (m_loading != loading) {
        m_loading = loading;
        emit loadingChanged();
    }
}

void ApiClient::sendRequest(
    const QString& endpoint,
    const QString& method,
    const QJsonObject& body,
    std::function<void(const QJsonObject&)> onSuccess,
    std::function<void(const QString&)> onError)
{
    QUrl url(m_serverUrl + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }

    QNetworkReply* reply = nullptr;

    if (method == "GET") {
        reply = m_networkManager.get(request);
    } else if (method == "POST") {
        reply = m_networkManager.post(request, QJsonDocument(body).toJson());
    } else if (method == "PUT") {
        reply = m_networkManager.put(request, QJsonDocument(body).toJson());
    } else if (method == "DELETE") {
        reply = m_networkManager.deleteResource(request);
    }

    if (!reply) {
        onError("Failed to create request");
        return;
    }

    setLoading(true);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        setLoading(false);
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString errorMsg = doc.object().value("error").toString();
            if (errorMsg.isEmpty()) {
                errorMsg = reply->errorString();
            }
            onError(errorMsg);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        onSuccess(doc.object());
    });
}

void ApiClient::sendArrayRequest(
    const QString& endpoint,
    const QString& method,
    const QJsonObject& body,
    std::function<void(const QJsonArray&)> onSuccess,
    std::function<void(const QString&)> onError)
{
    QUrl url(m_serverUrl + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }

    QNetworkReply* reply = nullptr;

    if (method == "GET") {
        reply = m_networkManager.get(request);
    } else if (method == "POST") {
        reply = m_networkManager.post(request, QJsonDocument(body).toJson());
    }

    if (!reply) {
        onError("Failed to create request");
        return;
    }

    setLoading(true);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        setLoading(false);
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString errorMsg = doc.object().value("error").toString();
            if (errorMsg.isEmpty()) {
                errorMsg = reply->errorString();
            }
            onError(errorMsg);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        onSuccess(doc.array());
    });
}

void ApiClient::login(const QString& username, const QString& password) {
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    sendRequest("/api/auth/login", "POST", body,
        [this](const QJsonObject& response) {
            AuthTokens tokens;
            tokens.accessToken = response["access_token"].toString();
            tokens.refreshToken = response["refresh_token"].toString();
            tokens.expiresIn = response["expires_in"].toInt();

            m_accessToken = tokens.accessToken;
            m_refreshToken = tokens.refreshToken;

            emit authenticationChanged();
            emit loginSuccess(tokens);
        },
        [this](const QString& error) {
            emit loginError(error);
        }
    );
}

void ApiClient::registerUser(const QString& username, const QString& email, const QString& password) {
    QJsonObject body;
    body["username"] = username;
    body["email"] = email;
    body["password"] = password;

    sendRequest("/api/auth/register", "POST", body,
        [this](const QJsonObject&) {
            emit registerSuccess();
        },
        [this](const QString& error) {
            emit registerError(error);
        }
    );
}

void ApiClient::logout() {
    m_accessToken.clear();
    m_refreshToken.clear();
    emit authenticationChanged();
}

void ApiClient::refreshToken() {
    QJsonObject body;
    body["refresh_token"] = m_refreshToken;

    sendRequest("/api/auth/refresh", "POST", body,
        [this](const QJsonObject& response) {
            AuthTokens tokens;
            tokens.accessToken = response["access_token"].toString();
            tokens.refreshToken = response["refresh_token"].toString();
            tokens.expiresIn = response["expires_in"].toInt();

            m_accessToken = tokens.accessToken;
            m_refreshToken = tokens.refreshToken;

            emit tokenRefreshed(tokens);
        },
        [this](const QString& error) {
            emit apiError(error);
        }
    );
}

void ApiClient::createPeer(const QString& deviceName, const QString& publicKey) {
    QJsonObject body;
    body["device_name"] = deviceName;
    body["protocol"] = "wireguard";
    body["public_key"] = publicKey;

    sendRequest("/api/vpn/peers", "POST", body,
        [this](const QJsonObject& response) {
            QJsonObject peerObj = response["peer"].toObject();

            PeerInfo peer;
            peer.id = peerObj["id"].toString();
            peer.deviceName = peerObj["device_name"].toString();
            peer.protocol = peerObj["protocol"].toString();
            peer.ipAddress = peerObj["wg_ip_address"].toString();
            peer.publicKey = peerObj["wg_public_key"].toString();
            peer.isActive = peerObj["is_active"].toBool();

            ServerConfig config;
            QString configStr = response["config"].toString();

            emit peerCreated(peer, config);
        },
        [this](const QString& error) {
            emit peerCreateError(error);
        }
    );
}

void ApiClient::getPeers() {
    sendArrayRequest("/api/vpn/peers", "GET", {},
        [this](const QJsonArray& response) {
            QList<PeerInfo> peers;

            for (const QJsonValue& val : response) {
                QJsonObject obj = val.toObject();
                PeerInfo peer;
                peer.id = obj["id"].toString();
                peer.deviceName = obj["device_name"].toString();
                peer.protocol = obj["protocol"].toString();
                peer.ipAddress = obj["wg_ip_address"].toString();
                peer.publicKey = obj["wg_public_key"].toString();
                peer.isActive = obj["is_active"].toBool();
                peers.append(peer);
            }

            emit peersLoaded(peers);
        },
        [this](const QString& error) {
            emit apiError(error);
        }
    );
}

void ApiClient::deletePeer(const QString& peerId) {
    sendRequest("/api/vpn/peers/" + peerId, "DELETE", {},
        [this, peerId](const QJsonObject&) {
            emit peerDeleted(peerId);
        },
        [this](const QString& error) {
            emit apiError(error);
        }
    );
}

void ApiClient::getPeerConfig(const QString& peerId) {
    QUrl url(m_serverUrl + "/api/vpn/peers/" + peerId + "/config");
    QNetworkRequest request(url);

    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }

    QNetworkReply* reply = m_networkManager.get(request);
    setLoading(true);

    connect(reply, &QNetworkReply::finished, this, [this, reply, peerId]() {
        setLoading(false);
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit apiError(reply->errorString());
            return;
        }

        QString config = QString::fromUtf8(reply->readAll());
        emit peerConfigLoaded(peerId, config);
    });
}

} // namespace obsidian
