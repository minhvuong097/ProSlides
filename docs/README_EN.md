<p align="center">
  <img src="../resources/icons/logo-icon-256.png" alt="ProSlides logo" width="128" height="128">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <strong>English</strong> | <a href="../README.md">Tiếng Việt</a>
</p>

<p align="center">
  <img alt="Build" src="https://img.shields.io/badge/build-passing-brightgreen">
  <img alt="Release" src="https://img.shields.io/badge/release-v0.10.0-blue">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-blue">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue">
  <img alt="Render" src="https://img.shields.io/badge/render-GPU%20%7C%20FFmpeg-brightgreen">
</p>

<p align="center">
  <a href="#features">Features</a> ·
  <a href="#install">Install</a> ·
  <a href="#quick-start">Quick start</a> ·
  <a href="#specs">Specs</a> ·
  <a href="#architecture">Architecture</a> ·
  <a href="#performance">Performance</a> ·
  <a href="#testing">Testing</a> ·
  <a href="#changelog">Changelog</a> ·
  <a href="#license">License</a>
</p>

Professional, lightweight worship presentation software written in **C++20 / Qt 6.12**.
It uses FreeShow's data model as its backbone and MxU Slides' minimal UX as its
interface direction — no Electron runtime, no Chromium overhead.

## Features

- FreeShow-compatible `.show` files (`[id, Show]`) with unknown-field preservation.
- SQLite FTS5 search with Vietnamese diacritic folding (`yeu` matches `yêu`).
- Automerge-style CRDT sync (LWW registers + RGA sequences) over WebSocket or relay.
- Timeline playback/editor, scripture (`.fsb`), service timers, templates.
- Overlays/scenes, audio playlists, MIDI/keyboard triggers.
- Hardware video decode (D3D11VA / DXVA2 / VideoToolbox / VAAPI) via FFmpeg.
- MP4 output recording, backups, Planning Center import.
- Phone remote on port 5510 with UDP discovery.

## Install

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

Output: `build\Release\ProSlides.exe`. See [build-windows.md](build-windows.md).

## Quick start

1. Paste lyrics like `[C]Amazing grace...` into the **Slides** panel and press **Add slide**.
2. Press **F5** to present on the audience display.
3. `Space` advances, `Ctrl+S` saves the `.show` file.
4. Enable **Remote** and control from your phone — see [protocol.md](protocol.md).

## Specs

| Item | Value |
|---|---|
| Language | C++20, Qt 6.12, QML |
| Show format | JSON `[id, Show]` (FreeShow compatible) |
| Database | SQLite 3 + WAL + FTS5 |
| Sync | Lamport/vector clocks, LWW + RGA CRDT |
| Remote | WebSocket 5510, UDP discovery 5514 |
| Video | FFmpeg HW decode → `QVideoSink`; Qt Multimedia fallback |
| Recording | MP4 H.264/MPEG-4, ~30 fps |
| Memory | ~40–80 MB idle (no Electron) |

## Architecture

See [architecture.md](architecture.md) and [protocol.md](protocol.md).

## Performance

Hardware D3D11VA decoding on Windows, a dedicated decoder thread, WAL + prepared
SQLite statements, and a lean QML UI keep the app responsive with multiple 4K loops.

## Testing

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## Changelog

### v0.10.0
Timeline, scripture, timers, templates, overlays/scenes, audio, triggers, MP4
recording, backups, Planning Center, cloud relay, new logo, GPL-3.0.

### v0.9.0
Initial platform: FreeShow data model, SQLite FTS5, CRDT, WebSocket remote, QML UI.

## License

**GPL-3.0** — see [LICENSE](../LICENSE).
