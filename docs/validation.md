# Підсумкова перевірка 0.1.0 — 2026-10-24

Функціональну версію Arthelm Beacon 0.1.0 завершено й перевірено локально. Нових відомих функціональних блокерів у перевіреному середовищі немає. Попередні записи нижче збережені як історія; зокрема старий збій GPU-тесту кліку виправлено.

## Виправлення та фінальний стан

- Точна причина старого збою — клік у невидимий центр порту при кадруванні високого вузького вікна. Клік по резиденції проходив. Тест тепер бере перетин області об’єкта, сцени та вікна, а не надсилає подію за їхні межі.
- CPU відкриває список процесів за CPU; Processes відкриває список за пам’яттю. Позначки залишаються доступними при повністю обрізаному порту чи резиденції. Реальні кліки перевірено на мінімальному та високому вузькому вікнах.
- Підключено обидві панорами 3840 × 2160, PNG-парусник, прозорі логотипи, повну денну/нічну палітру разом із підказками та секундний цифровий перехід. Напис In Development замінено на номер версії.
- `.SRCINFO` повторно згенеровано через `makepkg --printsrcinfo`; залежність `qt6-shadertools` синхронізовано з PKGBUILD.

## Середовище й результати

Arch Linux x86_64, GCC 16.2.1, Qt 6.12.0, CMake 4.4.4, Ninja 1.13.2, Wayland і XWayland/XCB; апаратний рендер OpenGL. Числа Qt Test включають init/cleanup.

| Перевірка | Результат |
| --- | --- |
| Чиста Release-збірка в `build-final` | Пройшла без попередніх об’єктів збірки |
| Core / GUI offscreen у чистій збірці | 17 / 12 PASS, обидва CTest-набори пройшли, 24.63 с |
| Повний GUI, Wayland/OpenGL, фінальні джерела | 12 PASS, 18.11 с |
| Повний GUI, XCB/OpenGL, фінальні джерела | 12 PASS, 18.56 с |
| Повний GUI, offscreen software, масштаб 200% | 12 PASS, 18.75 с |
| AddressSanitizer + UBSan, Debug, фінальні джерела | Обидва CTest-набори пройшли, 28.83 с; `detect_leaks=1`, жодних повідомлень про помилки чи витоки |
| Qt 6 qmllint для Theme, ActionButton, IconButton | Пройшов; повний QML-інтерфейс також завантажується без engine warnings у GUI-наборах |
| Desktop entry / синтаксис shell-скриптів | `desktop-file-validate` та `bash -n` пройшли |
| Повторне створення маски | Побітово тотожне наявній `scene-mask.png` |
| Вимірювання суміщення 4K-панорам | Шість локальних перевірок відтворили задокументовані зсуви; GPU використовує приблизну корекцію |
| Встановлення через DESTDIR | Пройшло; бінарник, desktop entry, іконка й ліцензії присутні |
| Повний цикл фінального makepkg | Збірка, `check()`, `package()` пройшли; тести 17 / 12 PASS, 24.58 с |
| Бінарник, витягнутий із фінального Arch-пакунка | `--smoke-test` з `/tmp` пройшов на software, Wayland і XCB |
| Візуальний перегляд | День/ніч 1600 × 1000 та мінімальне вікно 800 × 640 перевірено; screenshots оновлено |

GUI-набори перевіряють реальні метрики й історію, списки процесів, обидва сортування, усі діалогові вікна, доступність позначок, обрізані області кліку, паузу/відновлення, мінімізацію, фон логотипа, проміжні кольори та зміну напряму переходу. GPU-перевірки підтверджують рух моря й нерухомість контрольної ділянки даху. Код активного парусника завантажує саме `assets/art/sailboat.png`.

## Ресурси й стабільність

Виміряно лише запущений процес застосунку на Wayland. CPU наведено як частку одного логічного ядра; це не відсоток усієї системи й не виміряний FPS.

| Режим | CPU | RSS |
| --- | --- | --- |
| Анімації, 20.02 с після прогріву | 24.63% одного ядра | 304.62 MiB |
| Reduce decorative animation, 20.01 с | 1.05% одного ядра | 301.59 MiB |

Окремий 120-секундний запуск завершився штатно. RSS на 15/45/75/105 с: 304.11 / 304.30 / 304.48 / 304.30 MiB. У цьому інтервалі сталого зростання не спостерігалося.

## Артефакти та межі перевірки

`dist/` містить фінальний Arch-пакунок x86_64, архів джерел, `SHA256SUMS` і сирі журнали перевірок у `dist/validation/`. Пакунок побудовано з ізольованого локального Git-знімка робочих джерел; його revision/hash належить цьому знімку. Пакунок витягнуто й перевірено без встановлення до системи. Публікацію в upstream/AUR не виконували.

ARM/aarch64, мінімальний Qt 6.4 і збірку в чистому Arch chroot не перевіряли. На цьому Qt/Wayland залишаються платформні text-input warnings. QtTest також видає попередження про invalid metatype під час `QAbstractItemModelTester`; стек показує виклик усередині QtTest/QtCore. Звичайні тести та sanitizer-набори при цьому проходять. Ці попередження не приховано.

---

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

## Безперервна сцена — 08.10.2026

Поточний Release перевірено з Qt 6.11.2 / GCC 16.2.1, Hyprland/Wayland, Intel Iris Xe (ADL GT2), Mesa 26.2.4, фізичний scale 1. ShaderTools успішно зібрав новий шейдер; Vulkan headers у середовищі відсутні, Vulkan backend не перевірявся.

- `ctest --test-dir build --output-on-failure`: core і gui-offscreen пройшли, 15.97 с загалом. У GUI-тесті додано монотонний час, 400-мс блокування event loop, паузу/відновлення, маски нерухомої архітектури, зміну напрямку переходу з поточного стану, незалежність часу від метрик і resize. Core перевіряє обробку GNOME/KDE motion hints без перезапису ручного налаштування.
- Повний `QT_QPA_PLATFORM=wayland ./build/island-gui-tests`: 11 PASS з init/cleanup, 0 fail, 17.83 с. Перевірено справжні метрики, інтеракції, мінімізацію/відновлення, кадрування й рух. Додатково GPU-тест доводить зміну пікселів океану та піксельну стабільність контрольної ділянки даху при просуванні часу. Заморожений кадр залишається незмінним.
- Денний і нічний знімки 1600 × 950 переглянуто. Човен один, UI читається; у вузькому кадрі його розміщення адаптується, щоб корпус не обрізався. Місяць, гора й архітектура не мають деформації всього зображення.
- Збережено ліміти частинок, спільний годинник, кеш геометрії/буферів і обмеження частоти растрових перемальовувань. `git diff --check` і shell syntax check нового скрипту пройшли. Після уточнення маски опор причалу повторено обидва CTest-набори (16.17 с) та GPU-інтеграцію (10.03 с). Гарячу маску обмежено вище верхніх дахів, щоб колір теплих вікон не сприймався як лава.

Ресурси виміряно для видимого нічного застосунку, із закритою панеллю процесів, фактичний розмір вікна 931 × 974. Наведено CPU **одного логічного ядра**, не частку всіх 12 ядер. RSS — знімки процесу, не GPU VRAM і не гарантований пік.

| Перевірка | CPU | RSS |
| --- | ---: | ---: |
| Анімація після оптимізації, 20.01 с після прогрівання | 42.83% | 202.22 MiB |
| Анімація, 120-секундний запуск; CPU за інтервал 15–105 с | приблизно 43.2% | 208.52–208.96 MiB |
| Reduced motion, 20.01 с після прогрівання | 1.05% | 202.34 MiB |

У 120-секундному запуску RSS: 15 с — 208.52 MiB; 45 с — 208.58; 75 с — 208.96; 105 с — 208.64. Стійкого зростання в цьому короткому інтервалі не спостерігалося. Це не доводить відсутність витоків протягом годин або днів. Під час розробки перший анімований варіант споживав приблизно 67% ядра; частота перемальовування повільних растрових деталей знизила навантаження. Навантаження активної сцени все ще помітне й може відрізнятися на іншому обладнанні. FPS не вимірювався й не заявляється.

Повторення:

```sh
QT_QPA_PLATFORM=wayland scripts/measure-resources.sh ./build/arch-island animated
QT_QPA_PLATFORM=wayland scripts/measure-resources.sh ./build/arch-island reduced
QT_QPA_PLATFORM=wayland scripts/measure-scene-stability.sh ./build/arch-island
```

Останній скрипт виконує такий самий обмежений 120-секундний сценарій зі знімками RSS і CPU ticks. Його shell syntax перевірено; наведені вимірювання отримано попереднім еквівалентним локальним сценарієм. Налаштування для вимірювань ізольовані у тимчасовому каталозі.

Підтримку системних GNOME/KDE сигналів перевірено модульно; реальне перемикання системного параметра в окремих desktop environments не перевірено. Для цього оновлення не повторювали XWayland, фізичний HiDPI або мінімальний Qt 6.4. Локальні обмеження плоскої художньої сцени описані в [living-scene.md](living-scene.md).

## Arthelm redesign · 2026-10-21

- Release configure and build completed with CMake/Ninja and Qt 6.12.0.
- `ctest --test-dir build --output-on-failure`: 2/2 suites passed (core and GUI offscreen).
- The GUI interaction test also passed on the Wayland display with hardware rendering: island interactions, process drawer, settings, day/night modes, reduced motion, minimum size, and minimize/restore. The Wayland platform emitted text-input focus warnings; the test collected no QML engine warnings.
- Inspected actual night and compact daytime captures for the new colors, Pixel Sail branding, embedded fonts, and resource values. Offscreen software screenshots document the interface; they do not establish hardware effect quality.

The separate Arch Island / IN DEVELOPMENT header block was subsequently removed. Rebuilt successfully and reran `islandInteraction` on Wayland and offscreen software; both passed. The hardware test verifies that sea and town pixels change as the scene clock advances while roof pixels remain stable. The documented screenshot was refreshed.


## Arthelm Beacon layout — 2026-10-21

Implemented the header, expanded responsive navigation, five metric cards, scene badges and light system footer from `arthelm_beacon.png`. Added root filesystem usage, hostname/kernel/uptime and scoped monitoring alerts. Storage and alert navigation are covered by real click tests; storage values and uptime are checked in the GUI integration run.

Validation: Release build succeeded; all 12 core and 11 offscreen GUI cases passed. After correcting overlapping detail text at 800×640, `islandInteraction` passed again on offscreen software and Wayland hardware rendering. Inspected full-size daytime and 800×640 captures. No QML engine warnings were collected. Wayland still emits platform text-input focus warnings, and the core process-model test emits its existing invalid-type warning. Hardware scene motion and stable roof checks passed. `docs/arch-island.png` is the daytime Wayland capture.

The reference's new mountain architecture, light trails and exact artwork are not implemented. Existing day/night panoramas, spatial masks, shaders and animated objects are preserved. Alerts only report unavailable monitored data and root usage >=90%; Storage Used is filesystem occupancy, not SMART health. Footer shows the actual network interface instead of an unmeasured active-user count.

## Beacon scene and interactions — 2026-10-24

- Rebuilt the native application and embedded Beacon day/night art with CMake/Ninja, Qt 6.12.0.
- Full `ctest --test-dir build --output-on-failure`: 2/2 suites passed (17.61 s).
- Expanded GUI coverage checks all five scene badges stay within the viewport, including the 504×486 scene at the minimum application size. Real clicks exercise Storage, Alerts and Network from both navigation and scene badges. The process count is checked against live availability.
- Wayland hardware `islandInteraction`: passed (12.43 s), including water/town animation, stable roof pixels, pause/resume, process drawer, settings and resizing. No QML engine warnings; the existing Wayland text-input focus warnings remain.
- Inspected day/night and 800×640 screenshots. The updated README image is an actual offscreen/software capture, not the reference mockup.
- This update adapts the reference artwork to the existing shoreline geometry. Fine details can vary across the generated lighting variants. Storage remains filesystem occupancy; no SMART health or active-session count is claimed.

## Supplied 4K panorama integration — 2026-10-24

- Embedded `island.png` and `island_night.png`, both 3840 × 2160; rebuilt water/palm mask at 1920 × 1080 and moved the logical world to 1600 × 900 without stretching artwork.
- Retraced resident routes, harbor and residence hit areas, lighthouse glow/beam, scene connectors, boat placement and coastal effects. Mask tests cover architecture, docks, foreground rocks and open water. Resident tests cover all 18 inhabitants over repeated return trips.
- Native edge correlation reproduced the six documented offsets. GPU sampling applies a smooth approximate correction; software fallback uses the original supplied pair. Fine generated texture differences remain.
- Release configuration and build passed. `ctest --test-dir build --output-on-failure`: both suites passed, 25.17 s (core 6.38 s, offscreen GUI 18.78 s).
- Wayland hardware `islandInteraction` passed, 11.56 s. Animated water and town changed while the sampled church roof stayed pixel-identical; frozen frames stayed stable. Process drawers, settings, reduced motion, resize and minimize/restore passed with no QML engine warnings. Existing Wayland text-input platform warnings remain.
- Inspected offscreen software captures at 1600 × 1000 and 800 × 640, in day and night modes. Updated `docs/arch-island.png` and added `docs/arch-island-night.png`. The current Wayland display constrains window size, so its capture was used for GPU behavior rather than the full-size documentation preview.
- Fixed a pre-existing build reference to the absent, untracked `assets/arch-island.svg`: desktop installation now uses the existing computer icon under the expected `arch-island.svg` name.

- Temporary staged installation passed; its executable also passed `--smoke-test` from `/tmp` with the offscreen software backend, confirming embedded resources load independently of the working directory.

### Existing sailboat artwork

Replaced the scene's procedural boat mesh with the embedded transparent `assets/art/sailboat.png` image. The shared clock still drives gentle bobbing and roll, including pause/resume. Release rebuild passed; targeted offscreen tests `lightingTransitionKeepsState`, `continuousSailing` and `islandInteraction` passed (5 cases including setup/cleanup, 17.50 s). Inspected the full-size daytime capture and refreshed both documentation screenshots.

### Full night palette

Night mode now changes the header, footer, metrics panel, all metric cards, icon backgrounds, text, separators and chart accents. The existing day/night/automatic setting controls both scenery and chrome. Hardware rendering also removes the bright backing from the brand mark; software retains the original small raster mark.

The new `nightThemeCoversChrome` regression test checks actual rendered background brightness across the header, footer, metrics panel and cards over night → day → night transitions. It passed on Wayland hardware rendering (3 cases including setup/cleanup, 1.12 s), without QML warnings. Existing Wayland text-input platform warnings remain. Full-size software screenshots were inspected and refreshed.

The broader Wayland interaction run passed the new night palette and GPU animation checks but failed its scene residence click in the current narrow compositor-constrained window. Maximizing was not honored by the compositor. This is separate from theme switching; the same complete interaction test passed using offscreen software rendering.

Release rebuild and final CTest run passed: core 6.43 s, offscreen GUI 19.79 s, total 26.22 s. No failures in either suite.

### Logo backing matches its surface

Replaced the GPU-only logo shader with a cached `BrandMark` painted item that removes the raster's white matte in memory. Both header and navigation marks are transparent on hardware and software rendering; original brand bytes stay unchanged. Header ink follows the theme, and navigation ink stays light on its navy panel. The renderer runs only when the item needs painting, with the two processed variants cached once.

Release build passed. `nightThemeCoversChrome`, extended to compare the logo corner against the header's actual rendered background, passed on offscreen software (0.98 s) and Wayland hardware (1.20 s). The complete offscreen `islandInteraction` also passed. Inspected the daytime screenshot: neither header nor rail has a rectangular logo backing. Refreshed day/night captures. This supersedes the earlier software-logo limitation.

### Smooth theme transitions

The full palette and header logo now follow the panorama's single four-second `lightingBlend`, including reversals while a transition is still running. No independent per-widget timers are created. The header brand renderer accepts a continuous ink blend and uses additive premultiplied compositing in a small temporary buffer so opacity remains stable.

Release build passed. The regression test verifies intermediate panel colors against the scene's actual blend, checks that reversing mid-transition does not jump, then checks both settled palettes and the logo's transparent backing. Passed on offscreen software (9.76 s) and Wayland hardware (9.90 s), with no QML engine warnings. Final CTest: both suites passed, core 6.53 s and GUI 28.29 s, total 34.82 s.

### Digital stepped theme switch

Replaced the four-second smooth fade with a one-second driver mapped through irregular holds, short jumps and small rebounds. The scenery, full palette and logo remain synchronized. Three short pulse windows produce a 1–1.5 px chrome shift plus low-opacity cyan/magenta seams and thin scan fragments. The overlay does not accept input, is absent at both endpoints and adds no continuous animation. Reduced decorative animation uses the simple fade without pulses.

Release build passed. Updated transition assertions allow the new duration while checking shared intermediate palette/scene values and reversal continuity. Wayland theme test passed (3.72 s). Final CTest passed both suites: core 6.23 s, GUI 16.89 s, total 23.12 s.
