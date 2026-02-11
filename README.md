# ObsidianClient

Qt6/QML клиент для Obsidian VPN с поддержкой WireGuard.

## Особенности

- Генерация ключей WireGuard на стороне клиента (приватный ключ никогда не покидает устройство)
- Curve25519 криптография для ключей
- JWT аутентификация
- Управление несколькими устройствами (peers)
- Кроссплатформенность (Linux, macOS, Windows)

## Требования

- Qt 6.2+
- CMake 3.16+
- Компилятор с поддержкой C++20
- WireGuard (wg-quick) для подключения

## Сборка

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Запуск

```bash
./build/ObsidianClient
```

## Структура проекта

```
├── CMakeLists.txt
├── include/
│   ├── ApiClient.h      # HTTP клиент для API сервера
│   ├── ConfigManager.h  # Управление настройками
│   ├── KeyGenerator.h   # Мост между C++ и QML для генерации ключей
│   ├── VpnConnection.h  # Управление WireGuard подключением
│   └── WireGuardKeys.h  # Curve25519 криптография
├── src/
│   ├── main.cpp
│   ├── ApiClient.cpp
│   ├── ConfigManager.cpp
│   ├── VpnConnection.cpp
│   └── WireGuardKeys.cpp
└── qml/
    ├── main.qml         # Главное окно
    ├── LoginPage.qml    # Страница входа
    ├── MainPage.qml     # Главная страница
    ├── ConnectionView.qml   # Статус подключения
    ├── PeerListView.qml     # Список устройств
    └── SettingsPage.qml     # Настройки
```

## Безопасность

Приватные ключи WireGuard генерируются локально на устройстве и никогда не передаются на сервер. На сервер отправляется только публичный ключ.

## Документация

- [Архитектура](docs/ARCHITECTURE.md) — техническое описание компонентов
- [Руководство пользователя](docs/USER_GUIDE.md) — как использовать клиент
