# Yandex Gateway firmware (Telink B91 sampleGW) — minimal build repo

Минимальный репозиторий для воспроизводимой сборки кастомной прошивки
Zigbee-координатора (`sampleGW`, Telink TLSR9218/B91) под Yandex bootloader
(формат `.ybl`). Содержит **только наши правки** поверх стокового
`telink-semi/telink_zigbee_sdk` + CI, собирающий `.bin`/`.ybl` в GitHub Actions.

Стендовые скрипты (GPIO/SSH/XMODEM-заливка, IP, пароли) сюда **не входят**
и не нужны для сборки.

## Состав

```
overlay/tl_zigbee_sdk/   наши файлы: копируются поверх свежего SDK в CI
  CMakeLists.txt                       сгенерён cdt2cmake, пропатчен под CLI-сборку
  apps/zigbee/sampleGW/zb_tunnel.c     NEW: RAW APS-туннель, хук точки входа
  apps/zigbee/sampleGW/zb_ieee.[ch]    NEW: фикс IEEE + команда 0x0903/0x8903
  apps/zigbee/sampleGW/sampleGateway.c хук туннеля + фикс IEEE в user_app_init()
  apps/zigbee/sampleGW/app_ui.c        отключён скан кнопок (конфликт с UART)
  apps/zigbee/sampleGW/board_b91_dongle.h  UART0 PB2=TX / PB3=RX
  apps/zigbee/sampleGW/zcl_sampleGatewayCb.c  ответ на чтение атрибутов
                                       только через туннель 0x8902 (0x8100 выкл)
  apps/common/comm_cfg.h               ZBHCI_UART=1, BOOT_LOADER_MODE=1 (0x8000)
  proj/drivers/drv_hw.c                обход проверки размера flash (1 МБ модуль)
  platform/chip_b91/clock.c            PLL-lock без зависания (fallback RC_24M)
  stack/zigbee/zb_hci/zbhci.h          ID 0x0901/0x8901/0x8902, 0x0903/0x8903
  stack/zigbee/zb_hci/zbhciCmdProcess.c 0x0901 send, 0x8902 push, Mgmt_LQI на
                                       себя, диспетчер 0x0903
tools/mk_yandex2.py      упаковщик .bin -> .ybl (только stdlib)
scripts/apply-overlay.py копирует overlay на SDK, проверяет дрейф версий
.github/workflows/build.yml  CI: SDK + overlay + toolchain -> .bin/.ybl
sdk-ref.txt              зашитый коммит upstream SDK
```

Что делает прошивка: ZBHCI-over-UART координатор, прозрачный туннель
manufacturer-specific кластеров (`0x0901/0x8901/0x8902`), стабильный IEEE
(фикс в `zb_ieee.c`, смена на лету командой `0x0903`).

## Сборка

Локально и в CI собирается одинаково на **Linux** (`ubuntu-latest`). Тулчейн
`riscv32-elf-gcc 7.4.0 (nds32le-elf-mculib-v5f)` жёстко требуется prebuilt-стеком
(`libzb_coordinator.a`). В CI он берётся напрямую с релиза Andes
(`ast-v3_2_3-release-linux`, он же Telink RDS V3.2.3) — ничего грузить в релизы
своего репо не нужно, `cygwin` не нужен.

Вручную на Linux:

```bash
wget -q https://github.com/andestech/Andes-Development-Kit/releases/download/ast-v3_2_3-release-linux/nds32le-elf-mculib-v5f.txz
sudo tar -xf nds32le-elf-mculib-v5f.txz -C /opt
git clone https://github.com/telink-semi/telink_zigbee_sdk sdk
cd sdk && git checkout $(cat ../sdk-ref.txt) && cd ..
python scripts/apply-overlay.py overlay/tl_zigbee_sdk sdk/tl_zigbee_sdk
cmake -G "Unix Makefiles" -DTOOLCHAIN_PATH=/opt/nds32le-elf-mculib-v5f -S sdk/tl_zigbee_sdk -B sdk/tl_zigbee_sdk/cmake_build
cmake --build sdk/tl_zigbee_sdk/cmake_build --target sampleGW_b91 -j$(nproc)
python tools/mk_yandex2.py sdk/tl_zigbee_sdk/cmake_build/sampleGW_b91.bin
```

На Windows локально по-прежнему можно собирать с Telink IoT Studio
(`RDS V3.2.3\toolchains\nds32le-elf-mculib-v5f` + `cygwin\bin` в `PATH` и
`cmake -G "MinGW Makefiles"`), но в CI это не используется.

## Проверка результата

Джоба печатает строку пакера и размеры, артефакты — `sampleGW_b91_fw`
(`.bin`, `.ybl`, `.map`). Успех:

```
... BIN_SIZE=<N> crc_ok=True
```

* `crc_ok=True`;
* `BIN_SIZE + 4 == размер .ybl`, `.ybl == .bin + 4` (эталон: `206038 + 4 = 206042`).

## Заметки

* `SDK_REF` (по умолчанию из `sdk-ref.txt`) — коммит upstream, против которого
  снят overlay. Если `apply-overlay` упадёт с `expected stock file missing`,
  обнови ref на свежий master и перезапусти (полные файлы overlay конфликтов
  патчей не дают, но API стека мог уплыть — тогда правится конкретный файл).
* Фиксированный IEEE координатора: `overlay/.../sampleGW/zb_ieee.c`
  (`FIXED_IEEE_DISP`). Поменяй под свой модуль при желании; смена на лету —
  командой `0x0903` (8 байт IEEE в display-порядке).
* Лицензия: upstream SDK — Telink (см. `license_list.txt` в SDK, исходники под
  `TELINK_APACHE`); overlay-файлы, производные от SDK, несут исходные
  копирайт-заголовки Telink. Стоковая `gateway.ybl` Яндекса и даташиты сюда
  не входят и не публикуются.
