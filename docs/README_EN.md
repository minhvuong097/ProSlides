<p align="center">
  <img src="../resources/icons/logo-icon-256.png" alt="ProSlides" width="120" height="120">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <strong>English</strong> | <a href="../README.md">Tiếng Việt</a>
</p>

<p align="center">
  <a href="https://github.com/minhvuong097/Pro-slides/actions/workflows/build.yml"><img src="https://github.com/minhvuong097/Pro-slides/actions/workflows/build.yml/badge.svg" alt="build"></a>
  <img src="https://img.shields.io/badge/language-C%2B%2B20%20%7C%20Qt%206-blue" alt="language">
  <img src="https://img.shields.io/badge/license-GPL--3.0-blue" alt="license">
  <img src="https://img.shields.io/badge/status-in%20development-orange" alt="status">
  <img src="https://img.shields.io/badge/platform-Windows%20target-lightgrey" alt="platform">
  <a href="../CONTRIBUTING.md"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen" alt="PRs welcome"></a>
</p>

<p align="center">
  <a href="#features">Features</a> ·
  <a href="#install">Install</a> ·
  <a href="#quick-start">Quick start</a> ·
  <a href="#structure">Structure</a> ·
  <a href="#remote">Remote</a> ·
  <a href="#status">Status</a> ·
  <a href="#testing">Testing</a> ·
  <a href="#contributing">Contributing</a> ·
  <a href="#license">License</a> ·
  <a href="#brand">Trademark</a>
</p>

Worship presentation software written in C++20 / Qt 6, based on FreeShow's data
model with a minimal MxU Slides-style interface. This project is under active
development; there is no official release yet.

---

<a id="features"></a>
## Features

| Module | Purpose |
|---|---|
| `core` | Show/Slide/Layout/Item types, `.show` files as `[id, Show]`, undo/redo |
| `persistence` | SQLite + FTS5 indexing and search |
| `crdt` | LWW + RGA replication with Lamport/vector clocks |
| `remote` | WebSocket control, UDP discovery, FreeShow-style API |
| `media`, `output` | FFmpeg/Qt Multimedia video, output display, NDI, MP4 recording |
| `timeline`, `scripture`, `timers` | Timeline playback, `.fsb` Bible, service timers |
| `templates`, `overlays`, `audio`, `triggers` | Templates, scenes, audio playlists, MIDI/keyboard triggers |
| `backup`, `providers`, `cloud` | Backups, Planning Center import, CRDT relay client |
| `presentation`, `app` | Central controller and entry point |
| `qml` | Three-pane UI: Service Flow + Media / Slide Grid / Output Preview |

<a id="install"></a>
## Install

Requires Visual Studio 2022 (MSVC, C++20), CMake ≥ 3.25 and Qt 6 with Core, Gui,
Qml, Quick, QuickControls2, Sql, WebSockets, Network, Multimedia and Svg. FFmpeg
and the NDI SDK are optional.

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

CI builds with Qt 6.8 LTS (see `.github/workflows/build.yml`). Details:
[build-windows.md](build-windows.md).

<a id="quick-start"></a>
## Quick start

1. Paste lyrics into the Slides area (`[C]` marks chords) and press **Add slide**.
2. Press **F5** for the audience display.
3. Use `Space` / arrows to navigate and `Ctrl+S` to save.
4. Enable **Remote** in the top bar to control from a phone.

<a id="structure"></a>
## Structure

```
src/   core/ persistence/ crdt/ remote/ media/ output/ timeline/ scripture/
       timers/ templates/ overlays/ audio/ triggers/ recording/ backup/
       providers/ cloud/ presentation/ app/
qml/   QML interface
docs/  architecture, protocol, build
tests/ unit tests
.github/ CI workflow and issue templates
```

<a id="remote"></a>
## Remote

Protocol, API list and examples: [protocol.md](protocol.md). Data model and
schema: [architecture.md](architecture.md).

<a id="status"></a>
## Status

**In the source tree:** data model, SQLite/FTS5, CRDT, remote, QML UI, timeline,
scripture, timers, templates, overlays, audio, triggers, recording, backups,
Planning Center, cloud relay client — with unit tests.

**Not finished or not verified yet:** no official release and no real UI screenshot;
visual timeline keyframe easing; the relay server itself; zero-copy D3D11 → RHI
(frame is transferred to CPU once); macOS/Linux hardware testing.

<a id="testing"></a>
## Testing

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Tests are in `tests/tst_proslides.cpp`. Build/test results are reported by the
CI badge at the top.

<a id="contributing"></a>
## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md). Use the issue templates in
`.github/ISSUE_TEMPLATE/` for bugs and feature requests.

<a id="license"></a>
## License

**GPL-3.0** — see [LICENSE](../LICENSE).

<a id="brand"></a>
## Trademark

The name **ProSlides** and the logo assets in `resources/icons/` are brand
assets, © 2026 minhvuong097 and ProSlides contributors. GPL-3.0 covers source
code only and does not grant rights to the name or logo. See
[TRADEMARK.md](../TRADEMARK.md) and [NOTICE](../NOTICE).
