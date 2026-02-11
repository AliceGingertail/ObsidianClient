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
    // Prefer NetworkManager (no root needed)
    if (QFileInfo::exists("/usr/bin/nmcli") ||
        QFileInfo::exists("/usr/local/bin/nmcli")) {
        return true;
    }
    return QFileInfo::exists("/usr/bin/wg-quick") ||
           QFileInfo::exists("/usr/local/bin/wg-quick");
#elif defined(Q_OS_MACOS)
    return QFileInfo::exists("/usr/local/bin/wg-quick") ||
           QFileInfo::exists("/opt/homebrew/bin/wg-quick");
#elif defined(Q_OS_WIN)
    return QFileInfo::exists("C:/Program Files/WireGuard/wireguard.exe");
#else
    return false;
#endif
}

bool VpnConnection::hasNetworkManager() const {
    return QFileInfo::exists("/usr/bin/nmcli") ||
           QFileInfo::exists("/usr/local/bin/nmcli");
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
    m_process->start("wireguard.exe", {"/installtunnelservice", configPath});
#elif defined(Q_OS_LINUX)
    if (hasNetworkManager()) {
        // NetworkManager: import config and activate (no root needed)
        // First remove old connection if exists, then import fresh
        m_nmConnectionName = QFileInfo(configPath).baseName();
        QProcess cleanup;
        cleanup.start("nmcli", {"connection", "delete", m_nmConnectionName});
        cleanup.waitForFinished(3000);

        // Import the config file
        m_process->start("nmcli", {"connection", "import", "type", "wireguard", "file", configPath});
    } else {
        // Fallback: pkexec for graphical sudo prompt
        m_process->start("pkexec", {"wg-quick", "up", configPath});
    }
#elif defined(Q_OS_MACOS)
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
#elif defined(Q_OS_LINUX)
    if (hasNetworkManager()) {
        m_process->start("nmcli", {"connection", "down", m_nmConnectionName});
    } else {
        m_process->start("pkexec", {"wg-quick", "down", m_currentConfigPath});
    }
#elif defined(Q_OS_MACOS)
    m_process->start("wg-quick", {"down", m_currentConfigPath});
#endif
}

void VpnConnection::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit) {
        setError("WireGuard process crashed");
        return;
    }

    if (m_state == ConnectionState::Connecting) {
#ifdef Q_OS_LINUX
        if (hasNetworkManager() && exitCode == 0) {
            // Import succeeded, now activate the connection
            QProcess activate;
            activate.start("nmcli", {"connection", "up", m_nmConnectionName});
            activate.waitForFinished(10000);
            if (activate.exitCode() == 0) {
                setState(ConnectionState::Connected);
            } else {
                QString output = activate.readAllStandardError();
                setError("Failed to activate VPN: " + output);
            }
            return;
        }
#endif
        if (exitCode == 0) {
            setState(ConnectionState::Connected);
        } else {
            QString output = m_process->readAllStandardError();
            setError("Failed to connect: " + output);
        }
    } else if (m_state == ConnectionState::Disconnecting) {
#ifdef Q_OS_LINUX
        if (hasNetworkManager()) {
            // Also delete the NM connection on disconnect
            QProcess cleanup;
            cleanup.start("nmcli", {"connection", "delete", m_nmConnectionName});
            cleanup.waitForFinished(3000);
        }
#endif
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
    QString stdout = m_process->readAllStandardOutput();
    QString stderr = m_process->readAllStandardError();

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

#ifdef Q_OS_LINUX
    if (QFileInfo::exists("/usr/bin/nmcli")) {
        QProcess nmShow;
        nmShow.start("nmcli", {"connection", "show", m_nmConnectionName});
        nmShow.waitForFinished(3000);
        return QString::fromUtf8(nmShow.readAllStandardOutput());
    }
#endif

    QProcess wgShow;
    wgShow.start("wg", {"show"});
    wgShow.waitForFinished(3000);

    return QString::fromUtf8(wgShow.readAllStandardOutput());
}

} // namespace obsidian
