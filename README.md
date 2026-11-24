<p align="center">
  <img src="resources/icons/logo-icon-256.png" alt="ProSlides logo" width="128" height="128">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <a href="docs/README_EN.md">English</a> | <strong>Tiếng Việt</strong>
</p>

<p align="center">
  <img alt="Language" src="https://img.shields.io/badge/language-C%2B%2B20%20%7C%20Qt%206-blue">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-blue">
  <img alt="Status" src="https://img.shields.io/badge/status-in%20development-orange">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey">
</p>

<p align="center">
  <a href="#tính-năng">Tính năng</a> ·
  <a href="#cài-đặt">Cài đặt</a> ·
  <a href="#sử-dụng-nhanh">Sử dụng nhanh</a> ·
  <a href="#cấu-trúc">Cấu trúc</a> ·
  <a href="#remote">Remote</a> ·
  <a href="#kiểm-thử">Kiểm thử</a> ·
  <a href="#giấy-phép">Giấy phép</a>
</p>

Phần mềm trình chiếu worship viết bằng C++20 / Qt 6, lấy cấu trúc dữ liệu của FreeShow làm xương sống và giao diện tối giản kiểu MxU Slides. README này mô tả những gì đã có trong source; chưa có bản phát hành chính thức.

---

## Tính năng

Các module đã hiện thực trong `src/`:

- `core` — kiểu dữ liệu Show/Slide/Layout/Item, file `.show` dạng `[id, Show]`, undo/redo.
- `persistence` — SQLite + FTS5, ánh xạ và tìm kiếm slide.
- `crdt` — đồng bộ LWW + RGA với Lamport/vector clock.
- `remote` — WebSocket điều khiển, discovery UDP, API kiểu FreeShow.
- `media`, `video`, `output` — phát video (FFmpeg/Qt Multimedia), màn hình output, NDI, ghi hình MP4.
- `timeline`, `scripture`, `timers`, `templates`, `overlays`, `audio`, `triggers`, `backup`, `providers`, `cloud`.
- `qml` — giao diện 3 vùng: Service Flow + Media / Slide Grid / Output Preview.

## Cài đặt

Yêu cầu: Visual Studio 2022 (MSVC, C++20), CMake ≥ 3.25, Qt 6 (Core, Gui, Qml, Quick, QuickControls2, Sql, WebSockets, Network, Multimedia, Svg). FFmpeg và NDI SDK là tùy chọn.

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

Xem chi tiết tại [docs/build-windows.md](docs/build-windows.md).

## Sử dụng nhanh

1. Mở app, nhập lyric vào vùng Slides (cú pháp `[C]` cho hợp âm), bấm **Add slide**.
2. Bấm **Present (F5)** để mở cửa sổ trình chiếu.
3. `Space` / `←` / `→` để điều hướng slide, `Ctrl+S` để lưu.
4. Bật **Remote** trên thanh trên cùng để điều khiển từ điện thoại.

## Cấu trúc

```
src/
  core/ persistence/ crdt/ remote/ media/ output/
  timeline/ scripture/ timers/ templates/ overlays/
  audio/ triggers/ recording/ backup/ providers/
  cloud/ presentation/ app/
qml/        giao diện QML
docs/       tài liệu kiến trúc, giao thức, build
tests/      unit test
```

## Remote

Giao thức WebSocket và danh sách API: [docs/protocol.md](docs/protocol.md). Kiến trúc dữ liệu: [docs/architecture.md](docs/architecture.md).

## Kiểm thử

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Test nằm ở `tests/tst_proslides.cpp`. Lưu ý: mã nguồn chưa được biên dịch trong môi trường tạo repo (không có Qt/Windows SDK), cần build trên máy có Qt 6 để xác nhận.

## Giấy phép

**GPL-3.0** — xem [`LICENSE`](LICENSE).
