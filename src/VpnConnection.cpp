#include "VpnConnection.h"
#include <QFileInfo>

namespace obsidian {

VpnConnection::VpnConnection(QObject* parent)
    : QObject(parent)
    , m_process(std::make_unique<QProcess>(this))
{
    connect(m_process.get(), &QProcess::finished,
            this, &VpnConnection::onProcessFinished);
    connect(m_process.get(), &QProcess::errorOccurred,
            this, &VpnConnection::onProcessError);
    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &VpnConnection::onProcessOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &VpnConnection::onProcessOutput);
}

VpnConnection::~VpnConnection() {
    if (m_state == ConnectionState::Connected ||
        m_state == ConnectionState::Connecting) {
        disconnectVpn();
    }
}

void VpnConnection::setState(ConnectionState state) {
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);

        if (state == ConnectionState::Connected) {
            emit connected();
        } else if (state == ConnectionState::Disconnected) {
            emit disconnected();
        }
    }
}

void VpnConnection::setError(const QString& error) {
    m_errorMessage = error;
    emit errorMessageChanged();
    emit connectionError(error);
    setState(ConnectionState::Error);
}

bool VpnConnection::checkWireGuardAvailable() const {
#ifdef Q_OS_LINUX
    return QFileInfo::exists("/usr/bin/wg-quick") ||
           QFileInfo::exists("/usr/local/bin/wg-quick");
#elif defined(Q_OS_MACOS)
    return QFileInfo::exists("/usr/local/bin/wg-quick") ||
           QFileInfo::exists("/opt/homebrew/bin/wg-quick");
#elif defined(Q_OS_WIN)
    // Windows uses wireguard.exe
    return QFileInfo::exists("C:/Program Files/WireGuard/wireguard.exe");
#else
    return false;
#endif
}

void VpnConnection::connectVpn(const QString& configPath) {
    if (m_state == ConnectionState::Connected ||
        m_state == ConnectionState::Connecting) {
        setError("Already connected or connecting");
        return;
    }

    if (!checkWireGuardAvailable()) {
        setError("WireGuard is not installed");
        return;
    }

    if (!QFileInfo::exists(configPath)) {
        setError("Configuration file not found: " + configPath);
        return;
    }

    m_currentConfigPath = configPath;
    setState(ConnectionState::Connecting);

#ifdef Q_OS_WIN
    // Windows: use wireguard.exe /installtunnelservice
    m_process->start("wireguard.exe", {"/installtunnelservice", configPath});
#else
    // Linux/macOS: use wg-quick up
    // Note: requires sudo or proper permissions
    m_process->start("wg-quick", {"up", configPath});
#endif
}

void VpnConnection::disconnectVpn() {
    if (m_state != ConnectionState::Connected &&
        m_state != ConnectionState::Connecting) {
        return;
    }

    setState(ConnectionState::Disconnecting);

#ifdef Q_OS_WIN
    m_process->start("wireguard.exe", {"/uninstalltunnelservice", m_currentConfigPath});
#else
    m_process->start("wg-quick", {"down", m_currentConfigPath});
#endif
}

void VpnConnection::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit) {
        setError("WireGuard process crashed");
        return;
    }

    if (m_state == ConnectionState::Connecting) {
        if (exitCode == 0) {
            setState(ConnectionState::Connected);
        } else {
            QString output = m_process->readAllStandardError();
            setError("Failed to connect: " + output);
        }
    } else if (m_state == ConnectionState::Disconnecting) {
        setState(ConnectionState::Disconnected);
        m_currentConfigPath.clear();
    }
}

void VpnConnection::onProcessError(QProcess::ProcessError error) {
    QString errorMsg;

    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Failed to start WireGuard. Check permissions.";
        break;
    case QProcess::Crashed:
        errorMsg = "WireGuard process crashed";
        break;
    case QProcess::Timedout:
        errorMsg = "WireGuard process timed out";
        break;
    default:
        errorMsg = "Unknown error: " + QString::number(static_cast<int>(error));
        break;
    }

    setError(errorMsg);
}

void VpnConnection::onProcessOutput() {
    // Log output for debugging
    QString stdout = m_process->readAllStandardOutput();
    QString stderr = m_process->readAllStandardError();

    // Could emit signals for UI to display
    if (!stdout.isEmpty()) {
        qDebug() << "WireGuard stdout:" << stdout;
    }
    if (!stderr.isEmpty()) {
        qDebug() << "WireGuard stderr:" << stderr;
    }
}

QString VpnConnection::getConnectionInfo() const {
    if (m_state != ConnectionState::Connected) {
        return QString();
    }

    // Run wg show to get connection info
    QProcess wgShow;
    wgShow.start("wg", {"show"});
    wgShow.waitForFinished(3000);

    return QString::fromUtf8(wgShow.readAllStandardOutput());
}

} // namespace obsidian
