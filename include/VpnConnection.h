#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <memory>

namespace obsidian {

class VpnConnection : public QObject {
    Q_OBJECT

    Q_PROPERTY(ConnectionState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString currentPeerId READ currentPeerId NOTIFY currentPeerIdChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
        Error
    };
    Q_ENUM(ConnectionState)

    explicit VpnConnection(QObject* parent = nullptr);
    ~VpnConnection() override;

    ConnectionState state() const { return m_state; }
    QString currentPeerId() const { return m_currentPeerId; }
    QString errorMessage() const { return m_errorMessage; }

    // Connection management
    Q_INVOKABLE void connectVpn(const QString& configPath);
    Q_INVOKABLE void disconnectVpn();
    Q_INVOKABLE bool isConnected() const { return m_state == ConnectionState::Connected; }

    // Status
    Q_INVOKABLE QString getConnectionInfo() const;

signals:
    void stateChanged(ConnectionState state);
    void currentPeerIdChanged();
    void errorMessageChanged();
    void connected();
    void disconnected();
    void connectionError(const QString& error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();

private:
    void setState(ConnectionState state);
    void setError(const QString& error);
    bool checkWireGuardAvailable() const;

    ConnectionState m_state = ConnectionState::Disconnected;
    QString m_currentPeerId;
    QString m_currentConfigPath;
    QString m_errorMessage;
    std::unique_ptr<QProcess> m_process;
};

} // namespace obsidian
