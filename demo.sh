#!/usr/bin/env bash
# Показова прогонка шифратора кадру.
# Шифрує sample.png трьома різними seed (як у статті) і розшифровує назад.
set -e

cd "$(dirname "$0")"

# Збірка, якщо виконуваного файлу ще немає
if [ ! -x ./uav_cipher ]; then
    echo "Збираю..."
    g++ -std=c++17 -O2 -o uav_cipher main.cpp
fi

if [ ! -f sample.png ]; then
    echo "Немає sample.png — покладіть будь-яке зображення з такою назвою."
    exit 1
fi

echo "== Шифрування трьома seed =="
./uav_cipher encrypt sample.png encrypted_44257.png 44257
./uav_cipher encrypt sample.png encrypted_1234.png  1234
./uav_cipher encrypt sample.png encrypted_1111.png  1111

echo "== Розшифрування (seed 44257) =="
./uav_cipher decrypt encrypted_44257.png decrypted_44257.png 44257

echo
echo "Готово. Порівняйте sample.png, encrypted_*.png та decrypted_44257.png."
echo "decrypted_44257.png має збігатися з оригіналом."
