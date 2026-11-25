<p align="center">
  <img src="resources/icons/logo-icon-256.png" alt="ProSlides" width="120" height="120">
</p>

<h1 align="center">ProSlides</h1>

<p align="center">
  <a href="docs/README_EN.md">English</a> | <strong>Tiếng Việt</strong>
</p>

<p align="center">
  <a href="https://github.com/minhvuong097/Pro-slides/actions/workflows/build.yml"><img src="https://github.com/minhvuong097/Pro-slides/actions/workflows/build.yml/badge.svg" alt="build"></a>
  <img src="https://img.shields.io/badge/language-C%2B%2B20%20%7C%20Qt%206-blue" alt="language">
  <img src="https://img.shields.io/badge/license-GPL--3.0-blue" alt="license">
  <img src="https://img.shields.io/badge/status-in%20development-orange" alt="status">
  <img src="https://img.shields.io/badge/platform-Windows%20target-lightgrey" alt="platform">
  <a href="CONTRIBUTING.md"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen" alt="PRs welcome"></a>
</p>

<p align="center">
  <a href="#features">Tính năng</a> ·
  <a href="#install">Cài đặt</a> ·
  <a href="#quick-start">Sử dụng nhanh</a> ·
  <a href="#structure">Cấu trúc</a> ·
  <a href="#remote">Remote</a> ·
  <a href="#status">Trạng thái</a> ·
  <a href="#testing">Kiểm thử</a> ·
  <a href="#contributing">Đóng góp</a> ·
  <a href="#license">Giấy phép</a> ·
  <a href="#brand">Thương hiệu</a> ·
  <a href="SECURITY.md">Bảo mật</a>
</p>

**ProSlides** là nền tảng trình chiếu worship chuyên nghiệp được xây dựng bằng C++20 và Qt 6. Dự án kế thừa mô hình dữ liệu mở của FreeShow (show, slide, layout, timeline, remote), áp dụng tư duy giao diện tối giản, thao tác nhanh của MxU Slides, và tận dụng giải mã video phần cứng (D3D11VA/FFmpeg) để vận hành ổn định trên máy cấu hình tầm trung.

Mục tiêu: một ứng dụng gọn nhẹ, mở rộng được, tương thích dữ liệu hiện có, hỗ trợ trình chiếu đa màn hình, điều khiển từ điện thoại và đồng bộ đa thiết bị — không có chi phí runtime của Electron. Đây là dự án đang phát triển; chưa có bản phát hành chính thức.

---

<a id="features"></a>
## Tính năng

Các module đã có trong `src/`:

| Module | Vai trò |
|---|---|
| `core` | Kiểu Show/Slide/Layout/Item, file `.show` dạng `[id, Show]`, undo/redo |
| `persistence` | SQLite + FTS5, ánh xạ và tìm kiếm slide |
| `crdt` | Đồng bộ LWW + RGA với Lamport/vector clock |
| `remote` | WebSocket điều khiển, discovery UDP, API kiểu FreeShow |
| `media`, `output` | Video FFmpeg/Qt Multimedia, màn hình output, NDI, ghi hình MP4 |
| `timeline`, `scripture`, `timers` | Visual timeline editor (kéo block, resize, zoom, playhead), Kinh Thánh `.fsb`, hẹn giờ |
| `chords` | Chuyển hợp âm cả show, nhận diện key, slash chord |
| Service Flow | Tạo project, thêm/xóa item, mở và điều hướng theo service |
| Visual editor | Chế độ Editor: kéo item, đổi cỡ chữ/màu/căn chỉnh trên canvas |
| Audio mixer | Nhiều kênh audio: play/stop, mute/solo, volume, Routing cơ bản |
| Streaming/NDI | RTMP/HLS qua FFmpeg, NDI sender (cần NDI SDK), điều khiển từ remote |
| Stage display | Màn hình riêng cho ca sĩ: slide hiện tại, slide kế, notes, đồng hồ |
| `templates`, `overlays`, `audio`, `triggers` | Template, overlay/scene, playlist audio, MIDI/phím tắt |
| `backup`, `providers`, `cloud` | Sao lưu, Planning Center, relay CRDT |
| `presentation`, `app` | Controller trung tâm và entry point |
| `qml` | Giao diện 3 vùng: Service Flow + Media / Slide Grid / Output Preview |

<a id="install"></a>
## Cài đặt

Yêu cầu: Visual Studio 2022 (MSVC, C++20), CMake ≥ 3.25, Qt 6 (Core, Gui, Qml, Quick, QuickControls2, Sql, WebSockets, Network, Multimedia, Svg). FFmpeg và NDI SDK là tùy chọn.

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
powershell -File scripts/build-windows.ps1 -WithFFmpeg -WithTests
```

CI hiện build với Qt 6.8 LTS (xem `.github/workflows/build.yml`). Xem chi tiết tại [docs/build-windows.md](docs/build-windows.md).

<a id="quick-start"></a>
## Sử dụng nhanh

1. Mở app, nhập lyric vào vùng Slides (cú pháp `[C]` cho hợp âm), bấm **Add slide**.
2. Bấm **Present (F5)** để mở cửa sổ trình chiếu.
3. `Space` / `←` / `→` điều hướng slide, `Ctrl+S` lưu file `.show`.
4. `Ctrl+K` mở Quick Search (FTS5) để tìm slide/group/ghi chú và nhảy tới slide.
5. Bật **Remote** trên thanh trên cùng để điều khiển từ điện thoại.

<a id="structure"></a>
## Cấu trúc

```
src/
  core/ persistence/ crdt/ remote/ media/ output/
  timeline/ scripture/ timers/ templates/ overlays/
  audio/ triggers/ recording/ backup/ providers/
  cloud/ presentation/ app/
qml/        giao diện QML
docs/       kiến trúc, giao thức, build
tests/      unit test
.github/    CI workflow và issue template
```

<a id="remote"></a>
## Remote

Giao thức WebSocket, danh sách API và ví dụ: [docs/protocol.md](docs/protocol.md). Kiến trúc dữ liệu và schema: [docs/architecture.md](docs/architecture.md).

<a id="status"></a>
## Trạng thái

**Đã có trong source:** data model, SQLite/FTS5, CRDT, remote, QML UI, timeline, scripture, timers, templates, overlays, audio, trigger, recording, backup, Planning Center, cloud relay — kèm unit test.

**Chưa hoàn thiện / chưa kiểm chứng:**
- Chưa có bản phát hành và chưa có ảnh chụp giao diện thật (cần build trên Windows để chụp).
- Keyframe easing trực quan cho timeline chưa có.
- Cloud relay server là tùy triển khai ngoài; client đã sẵn sàng kết nối.
- Đường decode zero-copy D3D11 → RHI chưa hiện thực; đang transfer frame về CPU một lần.
- macOS/Linux mới ở mức build từ source, chưa test thiết bị thực tế.

<a id="testing"></a>
## Kiểm thử

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Unit test nằm ở `tests/tst_proslides.cpp`: `.show` round-trip, lyric/chord, CRDT convergence, FTS5 tiếng Việt, timeline, scripture, timer, template/overlay, trigger, backup. Kết quả build/test được báo cáo qua badge CI ở đầu README.

<a id="contributing"></a>
## Đóng góp

Xem [CONTRIBUTING.md](CONTRIBUTING.md). Bug và đề xuất tính năng dùng issue template trong `.github/ISSUE_TEMPLATE/`.

<a id="license"></a>
## Giấy phép

**GPL-3.0** — xem [`LICENSE`](LICENSE). ProSlides kế thừa kiến trúc dữ liệu từ FreeShow (GPL-3.0), nên bản phân phối phải giữ giấy phép copyleft và cung cấp mã nguồn khi phân phối binary.

<a id="brand"></a>
## Thương hiệu

Tên **ProSlides** và bộ logo trong `resources/icons/` là tài sản thương hiệu, © 2026 minhvuong097 và những người đóng góp dự án. GPL-3.0 chỉ áp dụng cho mã nguồn, **không** cấp quyền dùng tên/logo. Xem [`TRADEMARK.md`](TRADEMARK.md) và [`NOTICE`](NOTICE).

Báo cáo lỗ hổng bảo mật: xem [`SECURITY.md`](SECURITY.md) — vui lòng không mở issue công khai cho lỗ hổng.
