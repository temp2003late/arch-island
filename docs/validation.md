# Перевірка MVP — 05.10.2026

Середовище: Arch Linux x86_64, Qt 6.11.2, GCC 16.2.1, Ninja 1.13.2, Intel Core i5-1235U (12 логічних CPU), 15.34 GiB RAM, Hyprland/Wayland і XWayland. Під час початкової перевірки системного CMake не було: офіційний бінарний CMake 4.1.2 розпаковано у `/tmp/arch-island-tools/cmake-4.1.2-linux-x86_64` без зміни системних пакетів. Під час підготовки комітів уже доступний системний CMake 4.4.4; ним перевірено проміжні дерева Git.

## Виконано

- Release-збирання C++20 через CMake + Ninja.
- Qt Test: 9 тестових методів core плюс init/cleanup; усі пройшли. CPU з виключенням guest, RAM через MemAvailable, невалідні дані, мережевий фактичний інтервал, нуль/від’ємний інтервал, скидання, перше вимірювання, зміна та зникнення інтерфейсу, складні назви процесів, PID reuse, завершення, CPU понад 100%, доступ до файлів, QAbstractItemModelTester, сортування/обмеження моделі, QSettings в ізольованому файлі та обмежена історія на 60 вимірювань.
- GUI Qt Test offscreen/software: завантаження ресурсів, справжні показники, кліки на вулкан/порт, процеси після двох вимірювань, закриття панелі, налаштування, день/ніч, reduced motion, 800 × 640, мінімізація й відновлення. QML runtime warnings відсутні.
- Той самий GUI-тест із реальним Wayland-бекендом і з xcb/XWayland — пройшов.
- HiDPI 2× перевірено через `QT_SCALE_FACTOR=2` **offscreen**; фізичний дисплей у цьому середовищі має scale 1.
- Після візуального оновлення перевірено завантаження обох художніх PNG-шарів із Qt Resources і живі графіки. Знімок `docs/arch-island.png` зроблено після 62 секунд справжніх вимірювань, без демонстраційних даних. Тести Wayland, XWayland і HiDPI повторено для нового інтерфейсу.
- `desktop-file-validate packaging/arch-island.desktop` — пройшов.
- CMake install у `/tmp/arch-island-stage` з префіксом `/usr`; бінарник, desktop entry, іконка та LICENSE присутні.
- Встановлений бінарник запущено через Wayland і xcb із поточного каталогу `/tmp`; усі вбудовані ресурси завантажилися.
- `.SRCINFO` згенеровано `makepkg --printsrcinfo`; повторна генерація й `diff` підтвердили збіг.
- У тимчасовому Git-репозиторії зі знімком джерел виконано повний `makepkg` із локальним `git+file://` замість upstream. `pkgver()`, Release build, check, fakeroot package й створення `.pkg.tar.zst` пройшли. Це **не** перевірка upstream або clean chroot. Використано `--nodeps`, оскільки CMake є локальним інструментом, а не встановленим pacman-пакетом; усі потрібні Qt-бібліотеки встановлені.

Точні команди для доступного локального CMake:

```sh
export PATH="/tmp/arch-island-tools/cmake-4.1.2-linux-x86_64/bin:$PATH"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=wayland ./build/island-gui-tests
QT_QPA_PLATFORM=xcb ./build/island-gui-tests
QT_SCALE_FACTOR=2 QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software ./build/island-gui-tests
DESTDIR=/tmp/arch-island-stage cmake --install build --prefix /usr
(cd /tmp && QT_QPA_PLATFORM=wayland /tmp/arch-island-stage/usr/bin/arch-island --smoke-test)
(cd /tmp && QT_QPA_PLATFORM=xcb /tmp/arch-island-stage/usr/bin/arch-island --smoke-test)
./build/arch-island
```

CMake у `/tmp` — тимчасовий інструмент перевірки; для постійного використання встановіть системну залежність за README.

## Вимірювання ресурсів

Два видимі запуски Release-бінарника через Wayland, нічний режим, панель процесів закрита. Після 5 секунд прогрівання взято 20.01-секундну різницю utime+stime із `/proc/PID/stat`, поділену на `CLK_TCK` і монотонний інтервал `/proc/uptime`. RSS — один знімок `/proc/PID/status` наприкінці інтервалу, **не** пікова пам’ять.

| Режим | CPU, частка одного ядра | RSS |
| --- | ---: | ---: |
| Новий художній інтерфейс, анімації | 21.89% | 200.52 MiB |
| Новий художній інтерфейс, reduced motion | 1.00% | 192.11 MiB |
| Попередній векторний інтерфейс, анімації | 17.99% | 169.42 MiB |
| Попередній векторний інтерфейс, reduced motion | 0.55% | 162.28 MiB |

Відтворення (за замовчуванням — 30 секунд із автоматичним закриттям застосунку):

```sh
QT_QPA_PLATFORM=wayland scripts/measure-resources.sh ./build/arch-island animated
QT_QPA_PLATFORM=wayland scripts/measure-resources.sh ./build/arch-island reduced
```

Це короткі одиничні вимірювання на працюючому desktop, із Qt/драйверами/рендерингом у загальному споживанні процесу. Вони не доводять низького споживання й не є прогнозом для іншого обладнання. Окремо не профільовано відкриту панель процесів, мінімізований застосунок або тривалу роботу. Зменшена анімація суттєво знизила CPU саме у цих вимірюваннях.

## Залишилося перед AUR

Upstream Git поки не має опублікованих файлів. Власнику потрібно опублікувати джерела, перевірити HTTPS-клонування, повторити `makepkg` із незміненим upstream source та виконати clean-chroot build. aarch64, мінімальний Qt 6.4, окремий Xorg-сеанс, фізичний HiDPI-дисплей і різні desktop environments не перевірено. Назву в AUR треба перевірити повторно перед поданням. Нічого не пушено, не встановлено через pacman і не опубліковано в AUR.
