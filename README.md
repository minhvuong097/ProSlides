<p align="center">
  <img src="resources/icons/logo-icon-256.png" alt="ProSlides logo" width="128" height="128">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <a href="docs/README_EN.md">English</a> | <strong>Tiếng Việt</strong>
</p>

<p align="center">
  <img alt="Build" src="https://img.shields.io/badge/build-passing-brightgreen">
  <img alt="Release" src="https://img.shields.io/badge/release-v0.10.0-blue">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-blue">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue">
  <img alt="Render" src="https://img.shields.io/badge/render-GPU%20%7C%20FFmpeg-brightgreen">
</p>

<p align="center">
  <a href="#demo">Demo</a> ·
  <a href="#tính-năng">Tính năng</a> ·
  <a href="#cài-đặt">Cài đặt</a> ·
  <a href="#sử-dụng-nhanh">Sử dụng nhanh</a> ·
  <a href="#thông-số">Thông số</a> ·
  <a href="#kiến-trúc">Kiến trúc</a> ·
  <a href="#hiệu-năng">Hiệu năng</a> ·
  <a href="#kiểm-thử">Kiểm thử</a> ·
  <a href="#changelog">Changelog</a> ·
  <a href="#giấy-phép">Giấy phép</a>
</p>

Phần mềm trình chiếu worship chuyên nghiệp, nhẹ, viết bằng **C++20 / Qt 6.12**, lấy kiến trúc dữ liệu của **FreeShow** làm xương sống và UX tối giản của **MxU Slides** làm định hướng giao diện. Mục tiêu: chạy mượt trên máy cấu hình tầm trung với nhiều video 4K / NDI cùng lúc — không có chi phí của Electron.

---

## Demo

- Chạy nhanh sau khi build: `ProSlides.exe --help`, hoặc mở [hướng dẫn sử dụng nhanh](#sử-dụng-nhanh).
- Giao diện 3 vùng lấy cảm hứng từ MxU Slides: Service Flow + Media / Slide Grid / Output Preview.
- Remote trên điện thoại: `http://<ip-máy-tính>:5510` sau khi bật **Remote** trên top bar.

## Tính năng

| Nhóm | Tính năng |
|---|---|
| Soạn slide | Nhập lyric nhanh (`[C]` → chord tag), paste nhiều dòng, key, notes |
| Media | Thư viện quét thư mục, kéo thả vào slide, ảnh/video/audio |
| Layout | Nhiều layout, thêm/xóa/sắp xếp slide, transition |
| Timeline | Timeline action (ms) + duration (s), loop, playback engine + editor |
| Kinh Thánh | Đọc `.fsb` FreeShow, tìm kiếm, parse reference `John 3:16`, tạo slide |
| Trình chiếu | Màn hình audiences fullscreen, confidence monitor, clear/restore |
| Remote | WebSocket :5510, UDP discovery :5514, API tương thích FreeShow |
| Tìm kiếm | SQLite FTS5, `unicode61 remove_diacritics 2` (tìm `yeu` ra `yêu`) |
| Đồng bộ | CRDT LWW + RGA, Lamport/vector clock, relay qua WebSocket |
| Video | FFmpeg D3D11VA/DXVA2 (Windows), VideoToolbox (macOS), VAAPI (Linux) |
| Hẹn giờ | Timer đếm ngược/đồng hồ, cảnh báo màu, điều khiển từ remote |
| Template | Đọc `templates.json` FreeShow, áp dụng style/background vào slide |
| Overlay/Scene | Đọc `overlays.json`/`scenes.json`, bật/tắt overlay, chuyển scene |
| Audio | Playlist backing track, play/pause/next, volume/mute, seek |
| Trigger | MIDI note/CC và phím tắt → API action (`triggers.json`) |
| Ghi hình | Ghi output ra MP4 qua FFmpeg (H.264/MPEG-4), điều khiển từ remote |
| Backup | Snapshot `Shows/` + config + DB vào `Backups/`, restore & prune |
| Planning Center | Fetch plans qua REST API, import thành service project |
| Cloud sync | Relay CRDT qua WebSocket, đồng bộ nhiều máy |

## Cài đặt

Yêu cầu:
- Visual Studio 2022 (MSVC, C++20), CMake ≥ 3.25
- **Qt 6.12** (MSVC 64-bit): `Core, Gui, Qml, Quick, QuickControls2, Sql, WebSockets, Network, Multimedia, Svg`
- FFmpeg SDK (tùy chọn nhưng khuyến nghị) — D3D11VA
- NDI SDK v5+ (tùy chọn)

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

Hoặc xem [hướng dẫn build chi tiết](docs/build-windows.md). Kết quả: `build\Release\ProSlides.exe`.

## Sử dụng nhanh

1. Chạy app, ở vùng **Slides** paste lyric có `[C]Amazing grace...`, bấm **Add slide**.
2. Bấm **Present (F5)** → cửa sổ output fullscreen trên màn hình audiences.
3. `Space` → slide kế, `←/→` → điều hướng, `Ctrl+S` → lưu `.show`.
4. Bật **Remote** ở top bar → dùng điện thoại gọi `next_slide` (xem [protocol](docs/protocol.md)).
5. `start_timeline`, `name_start_timer`, `start_scene`, `start_output_recording` đều gọi được từ remote.

## Thông số

| Hạng mục | Giá trị |
|---|---|
| Ngôn ngữ | C++20, Qt 6.12, QML |
| Định dạng show | JSON `[id, Show]` tương thích FreeShow |
| Cơ sở dữ liệu | SQLite 3 + WAL + FTS5 (`unicode61 remove_diacritics 2`) |
| Đồng bộ | Automerge-style CRDT (LWW register + RGA sequence) |
| Remote | WebSocket 5510, UDP discovery 5514 |
| Video | FFmpeg HW decode → swscale → `QVideoSink`; fallback Qt Multimedia |
| Output | QML fullscreen window, NDI (tùy chọn) |
| Ghi hình | MP4 H.264/MPEG-4, ~30 fps |
| Bộ nhớ | ~40–80 MB khi chạy (không dùng Electron) |

## Kiến trúc

```
QML UI (MxU-style 3 vùng)
        │ Q_PROPERTY / Q_INVOKABLE
PresentationController
 ├─ ShowDocument / ShowFile        (undo/redo, .show [id, Show])
 ├─ ShowRepository + SQLite FTS5
 ├─ crdt::Document / SyncSession
 ├─ RemoteServer / CloudSyncClient
 ├─ Timeline / Scripture / Timers / Templates / Overlays
 ├─ Audio / Triggers / Recorder / Backup / Planning Center
 └─ OutputManager (audience display + NDI)
```

Xem chi tiết tại [docs/architecture.md](docs/architecture.md) và [docs/protocol.md](docs/protocol.md).

## Hiệu năng

- Giải mã video phần cứng **D3D11VA** trên Windows (DXVA2 làm fallback), giảm tải CPU khi phát nhiều video 4K.
- Decode chạy trên thread riêng, đẩy frame qua `QVideoSink` — không block UI thread.
- SQLite `WAL + cache 20MB + prepared statements`, FTS5 external-content cho slide.
- Không có runtime Electron/Chromium: RAM khởi động thấp, output ổn định.
- Ghi hình dùng encoder phần cứng/kết hợp FFmpeg, không cần readback full-frame về CPU nhiều lần.

## Kiểm thử

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Bộ test trong `tests/tst_proslides.cpp` phủ: `.show` round-trip, lyric/chord parsing, CRDT convergence, FTS5 tiếng Việt, timeline trigger, scripture reference, timer lifecycle, template/overlay JSON, trigger mapping, backup snapshot.

## Changelog

### v0.10.0
- Timeline playback engine + editor, scripture provider, timers, templates.
- Overlay/scene manager, audio playlist, MIDI/keyboard triggers.
- MP4 output recording, backup/restore, Planning Center provider, cloud CRDT relay.
- Logo mới, README song ngữ, giấy phép GPL-3.0.

### v0.9.0
- Phiên bản nền tảng: data model FreeShow, SQLite FTS5, CRDT, WebSocket remote, QML UI, FFmpeg/D3D11VA.

## Giấy phép

**GPL-3.0** — xem [`LICENSE`](LICENSE). ProSlides kế thừa kiến trúc dữ liệu từ FreeShow (cũng GPL-3.0), nên bản phân phối phải giữ nguyên giấy phép copyleft và cung cấp mã nguồn khi phân phối binary.
