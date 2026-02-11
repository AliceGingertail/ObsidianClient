#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "ApiClient.h"
#include "ConfigManager.h"
#include "VpnConnection.h"
#include "KeyGenerator.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    app.setOrganizationName("ObsidianVPN");
    app.setOrganizationDomain("obsidian.vpn");
    app.setApplicationName("ObsidianClient");
    app.setApplicationVersion("1.0.0");

    // Create core objects
    obsidian::ConfigManager configManager;
    obsidian::ApiClient apiClient;
    obsidian::VpnConnection vpnConnection;
    obsidian::KeyGenerator keyGenerator;

    // Set server URL from config
    apiClient.setServerUrl(configManager.serverUrl());

    // Load saved tokens if available
    auto tokens = configManager.loadTokens();
    if (tokens) {
        apiClient.setTokens(tokens->first, tokens->second);
    }

    // Connect config manager to API client
    QObject::connect(&configManager, &obsidian::ConfigManager::serverUrlChanged,
                     [&]() { apiClient.setServerUrl(configManager.serverUrl()); });

    // Save tokens when login succeeds
    QObject::connect(&apiClient, &obsidian::ApiClient::loginSuccess,
                     [&](const obsidian::AuthTokens& tokens) {
                         configManager.saveTokens(tokens.accessToken, tokens.refreshToken);
                     });

    // Clear tokens on logout
    QObject::connect(&apiClient, &obsidian::ApiClient::authenticationChanged,
                     [&]() {
                         if (!apiClient.isAuthenticated()) {
                             configManager.clearTokens();
                         }
                     });

    QQmlApplicationEngine engine;

    // Expose objects to QML
    engine.rootContext()->setContextProperty("configManager", &configManager);
    engine.rootContext()->setContextProperty("apiClient", &apiClient);
    engine.rootContext()->setContextProperty("vpnConnection", &vpnConnection);
    engine.rootContext()->setContextProperty("keyGenerator", &keyGenerator);

    // Register types for QML
    qmlRegisterUncreatableType<obsidian::VpnConnection>(
        "Obsidian", 1, 0, "VpnConnection",
        "VpnConnection is provided by the application");

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
