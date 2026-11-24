# ProSlides

Phần mềm trình chiếu worship chuyên nghiệp, nhẹ, viết bằng **C++20 / Qt 6.12**, lấy
kiến trúc dữ liệu của **FreeShow** làm xương sống và UX tối giản của **MxU Slides**
làm định hướng giao diện.

Mục tiêu: chạy mượt trên máy cấu hình tầm trung với nhiều video 4K / NDI cùng lúc —
không có chi phí của Electron.

---

## 1. Kiến trúc tổng quan

```
┌──────────────────────────────────────────────────────────────────┐
│  QML UI (MxU-style, 3 vùng)                                      │
│  TopBar | ServiceFlow + Media | SlideGrid + Quick Add | Output   │
└───────────────────┬──────────────────────────────────────────────┘
                    │ Q_PROPERTY / Q_INVOKABLE
┌───────────────────▼──────────────────────────────────────────────┐
│  PresentationController  (ps_presentation)                       │
│  ├─ ShowDocument   (undo/redo, thao tác slide/layout)            │
│  ├─ ShowRepository (SQLite FTS5 + file .show [id, Show])         │
│  ├─ crdt::Document + SyncSession (Automerge-style)               │
│  ├─ RemoteServer   (WebSocket 5510, API FreeShow)                │
│  ├─ MediaLibrary / Thumbnailer                                   │
│  └─ OutputManager  (audience display, NDI)                       │
└───────┬──────────────┬──────────────┬──────────────┬─────────────┘
        │              │              │              │
    ps_core        ps_persistence  ps_crdt/ps_remote  ps_media/ps_output
   Types.h/.cpp    schema.sql       Operation.cpp    FFmpeg (D3D11VA)
   ShowFile        Database.cpp     Document.cpp     QtMultimedia
   ShowDocument    ShowRepo.cpp     SyncSession.cpp  NdiOutput
```

### Nguyên tắc thiết kế

- **JSON là nguồn chân lý.** Mỗi show lưu dưới dạng file `.show` tương thích
  FreeShow: mảng JSON `[showId, ShowObject]`. SQLite chỉ là *projection* để
  truy vấn nhanh và FTS5.
- **Không mất dữ liệu khi đọc file lạ.** Field không nhận diện được trong
  `Item` được giữ trong `Item::extra` và ghi ngược lại khi save.
- **CRDT là tầng đồng bộ, không phải tầng lưu trữ.** Mỗi thao tác tạo
  operation có Lamport timestamp; peer đồng bộ bằng vector clock.
- **Video tách backend.** `VideoEngine` là interface; FFmpeg (hardware decode)
  và Qt Multimedia là hai implementation.

---

## 2. Build trên Windows

Yêu cầu:
- Visual Studio 2022 (MSVC, C++20)
- CMake ≥ 3.25
- **Qt 6.12** (MSVC 2019/2022 64-bit): `Core, Gui, Qml, Quick, QuickControls2,
  Sql, WebSockets, Network, Multimedia, Svg`
- FFmpeg SDK (tùy chọn, khuyến nghị) — cho D3D11VA
- NDI SDK v5/v6 (tùy chọn)

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
      -DPS_ENABLE_FFMPEG=ON -DPS_ENABLE_NDI=OFF
cmake --build build --config Release --parallel
```

Script tiện lợi: `powershell -File scripts/build-windows.ps1`

Chạy: `build\Release\ProSlides.exe [file.show]`

---

## 3. Tính năng chính

| Nhóm | Tính năng |
|---|---|
| Soạn slide | Nhập lyric nhanh (`[C]` → chord tag), paste nhiều dòng, key, notes |
| Media | Thư viện quét thư mục, kéo thả vào slide, ảnh/video/audio |
| Layout | Nhiều layout, thêm/xóa/sắp xếp slide, transition |
| Timeline | Timeline action (ms) + duration (s), loop |
| Trình chiếu | Màn hình audiences fullscreen, confidence monitor, clear/restore |
| Remote | WebSocket :5510, UDP discovery :5514, API tương thích FreeShow |
| Tìm kiếm | SQLite FTS5, `unicode61 remove_diacritics 2` (tìm `yeu` ra `yêu`) |
| Đồng bộ | CRDT LWW + RGA, Lamport/vector clock, chạy trên cùng kênh remote |
| Video | FFmpeg D3D11VA/DXVA2 (Windows), VideoToolbox (macOS), VAAPI (Linux) |

---

## 4. Tài liệu chi tiết

- `docs/architecture.md` — data model ánh xạ từ FreeShow, schema DB, CRDT
- `docs/protocol.md` — giao thức WebSocket/API
- `docs/build-windows.md` — hướng dẫn build & deploy chi tiết

---

## 5. Phạm vi hiện tại

Bộ source này là **nền tảng hoàn chỉnh, biên dịch được**: data model, persistence,
CRDT, remote, video backend, output và UI đã được hiện thực. Các phần nâng cao
(timeline editor trực quan, scripture/plugin providers, MIDI/HID, recording)
được dành riêng ở roadmap — xem `docs/architecture.md`.
