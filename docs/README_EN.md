<p align="center">
  <img src="../resources/icons/logo-icon-256.png" alt="ProSlides logo" width="128" height="128">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <strong>English</strong> | <a href="../README.md">Tiếng Việt</a>
</p>

<p align="center">
  <img alt="Language" src="https://img.shields.io/badge/language-C%2B%2B20%20%7C%20Qt%206-blue">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-blue">
  <img alt="Status" src="https://img.shields.io/badge/status-in%20development-orange">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey">
</p>

<p align="center">
  <a href="#features">Features</a> ·
  <a href="#install">Install</a> ·
  <a href="#quick-start">Quick start</a> ·
  <a href="#layout">Layout</a> ·
  <a href="#remote">Remote</a> ·
  <a href="#testing">Testing</a> ·
  <a href="#license">License</a>
</p>

Worship presentation software written in C++20 / Qt 6, using FreeShow's data
model as its backbone and a minimal MxU Slides-style interface. This README
describes what exists in the source tree; there is no official release yet.

## Features

Modules under `src/`:

- `core` — Show/Slide/Layout/Item types, `.show` files as `[id, Show]`, undo/redo.
- `persistence` — SQLite + FTS5 indexing and search.
- `crdt` — LWW + RGA replication with Lamport/vector clocks.
- `remote` — WebSocket control, UDP discovery, FreeShow-style API.
- `media`, `output` — video playback (FFmpeg/Qt Multimedia), output display, NDI, MP4 recording.
- `timeline`, `scripture`, `timers`, `templates`, `overlays`, `audio`, `triggers`, `backup`, `providers`, `cloud`.
- `qml` — three-pane UI: Service Flow + Media / Slide Grid / Output Preview.

## Install

Requires Visual Studio 2022 (MSVC, C++20), CMake ≥ 3.25, Qt 6. FFmpeg and NDI SDK are optional.

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

See [build-windows.md](build-windows.md).

## Quick start

1. Paste lyrics into the Slides area (`[C]` marks chords), press **Add slide**.
2. Press **F5** to open the presentation display.
3. Use `Space` / arrows to navigate, `Ctrl+S` to save.
4. Enable **Remote** in the top bar to control from a phone.

## Layout

```
src/  core/ persistence/ crdt/ remote/ media/ output/ timeline/ scripture/
      timers/ templates/ overlays/ audio/ triggers/ recording/ backup/
      providers/ cloud/ presentation/ app/
qml/  QML interface
docs/ architecture, protocol, build
tests/ unit tests
```

## Remote

Protocol and API list: [protocol.md](protocol.md). Data model: [architecture.md](architecture.md).

## Testing

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Tests live in `tests/tst_proslides.cpp`. The source has not been compiled in the
environment where this repo was created (no Qt/Windows SDK); build with Qt 6 to verify.

## License

**GPL-3.0** — see [LICENSE](../LICENSE).
