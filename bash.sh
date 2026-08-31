#!/bin/bash
# 1. Создаем структуру папок
mkdir -p share/glib-2.0/schemas
mkdir -p share/icons

# 2. Копируем GLib схемы (без них приложение упадет с ошибкой GSettings)
cp /mingw64/share/glib-2.0/schemas/gschemas.compiled share/glib-2.0/schemas/

# 3. Копируем базовую тему иконок (Adwaita / hicolor)
cp -r /mingw64/share/icons/Adwaita share/icons/
cp -r /mingw64/share/icons/hicolor share/icons/
