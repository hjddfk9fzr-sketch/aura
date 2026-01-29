# aura

Minimal experimental AArch64 hobby OS (UEFI + kernel).

## Что сделано
- Добавлен загрузчик UEFI (`boot/boot.c`): умеет читать `kernel.elf`, загружать сегменты (ELF64 PT_LOAD), получать информацию о framebuffer через GOP и передавать `BootInfo` в `kernel`.
- Ядро (`kernel/core/kernel.c`) теперь принимает указатель `BootInfo` (передаётся в x0) и использует параметры framebuffer вместо жёсткого хардкода.
- `kernel/video/framebuffer.c` использует реальные параметры (base, width, height, pitch).

## Быстрая сборка
- Сборка ядра: `make` (создаёт `kernel.elf`).

> Примечание: `boot/boot.c` — UEFI-приложение. Чтобы собрать `boot.efi` и запустить в QEMU, нужно установить EDK2/gnu-efi или собрать с помощью EDK II (см. ниже).

## Сборка UEFI-загрузчика (рекомендации)
1. Установите EDK II (AArch64) или библиотеку `gnu-efi` для aarch64.
2. С помощью `gnu-efi` можно собрать таким образом (пример, зависит от вашей системы):

   - Скомпилировать `boot.c` в объект:
     `aarch64-linux-gnu-gcc -fshort-wchar -fno-stack-protector -fPIC -I /usr/include/efi -c boot/boot.c -o boot.o`
   - Создать UEFI-приложение (пример, требует `gnuefi`):
     `ld -Bsymbolic -Ttext 0 -shared -o boot.so boot.o -lgnuefi` и затем `objcopy -O binary boot.so boot.efi` (варианты зависят от вашей средой).

3. Создайте FAT-образ с `EFI/BOOT/BOOTAA64.EFI` и `kernel.elf` рядом:
   - `mkdir -p mnt/EFI/BOOT`
   - `cp boot.efi mnt/EFI/BOOT/BOOTAA64.EFI`
   - `cp kernel.elf mnt/`
   - `mkfs.vfat -n AURABOOT image.img` и `mcopy -i image.img mnt/* ::` (варианты зависят от `mtools`)

## Запуск в QEMU (пример)
- Установите QEMU и EDK2 AArch64 firmware (`QEMU_EFI.fd` / `AAVMF` for aarch64).
- Запуск:
  `qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1024 -nographic -bios /path/to/QEMU_EFI.fd -drive file=image.img,format=raw,if=none,id=hd0 -device virtio-blk-device,drive=hd0`

## Дальше
- Проверить сборку `boot.efi` и образа для QEMU.
- Тщательно протестировать загрузчик и корректность передачи `BootInfo`.
- Добавить в `Makefile` автоматизацию сборки `boot.efi` и `make run`.

Если хочешь, могу: 1) добавить автоматическое правило сборки `boot.efi` (gnu-efi/EDK2) и `make run` для QEMU, или 2) сначала создать минимальный тестный образ и запустить его в QEMU у тебя в контейнере. Что предпочитаешь?
Загрузчик: сделать UEFI загрузчик, который загружает ELF‑ядро и передаёт данные (framebuffer, memory map, ACPI/GIC info). ✅
Базовое ядро: инициализация стека/сегментов, установка страниц (MMU), простой аллокатор. ✅
Драйверы ранней стадии: UART (консоль), timer (таймер/тик), GIC (прерывания).
Многозадачность: планировщик, контекстные переключения, syscall-интерфейс.
Драйвер видео/ввод: Framebuffer/GOP, клавиатура/мышь.
Пользовательский уровень: init, процессы, файловая система (minfs), IPC.
Графическая подсистема: compositor/window server → GUI toolkit → приложения.
Инструменты: сборка cross‑toolchain, QEMU образ, тесты и CI.