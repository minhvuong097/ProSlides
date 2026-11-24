# Build & Deploy trên Windows

## 1. Chuẩn bị

| Phần mềm | Ghi chú |
|---|---|
| Visual Studio 2022 | workload **Desktop development with C++** |
| CMake ≥ 3.25 | `winget install Kitware.CMake` |
| Qt 6.12 MSVC 64-bit | Components: `Core, Gui, Qml, Quick, QuickControls2, Sql, WebSockets, Network, Multimedia, Svg` |
| FFmpeg SDK (tùy chọn) | `libavcodec/libavformat/libavutil/swscale` — cần cho D3D11VA |
| NDI SDK v5+ (tùy chọn) | đặt `NDI_SDK_DIR` tới thư mục SDK |

Nếu dùng Qt Maintenance Tool, cài cả **Qt Quick Controls 2** và **Qt Multimedia**.

## 2. Build bằng script

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.12.0\msvc2019_64"
.\scripts\build-windows.ps1 -WithFFmpeg -WithTests
```

## 3. Build thủ công

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.12.0\msvc2019_64" `
  -DPS_ENABLE_FFMPEG=ON -DPS_BUILD_TESTS=ON

cmake --build build --config Release --parallel

ctest --test-dir build -C Release --output-on-failure
```

Kết quả: `build\Release\ProSlides.exe`. `windeployqt` được gọi tự động sau build
(POST_BUILD) để copy DLL Qt, plugin image/qml, và runtime MSVC.

## 4. FFmpeg

CMake tìm FFmpeg qua `find_package(FFmpeg)`. Nếu dùng build tự compile, set:

```
-DFFmpeg_DIR=<prefix>/lib/cmake/FFmpeg
```

Khi tìm thấy, macro `PS_HAS_FFMPEG=1` được định nghĩa và backend hardware decode
được biên dịch. Nếu không, `PS_HAS_FFMPEG` không tồn tại → code đi vào stub và
ứng dụng vẫn dùng được Qt Multimedia backend.

## 5. Chạy & dữ liệu

```
ProSlides.exe [path/to/show.show]
```

Dữ liệu mặc định tại `%APPDATA%\ProSlides\` (xem `docs/architecture.md`).
Để chạy portable, đặt file `portable.txt` cạnh exe (mở rộng: `defaultPath()` có thể
được chỉnh để trả về thư mục exe nếu có file này).

## 6. Kiểm tra nhanh

1. Chạy app → tab **Slides**, paste lyric có `[C]...`, bấm **Add slide**.
2. Bấm **Present (F5)** → cửa sổ output fullscreen.
3. Bật **Remote** trên top bar → mở `http://<ip>:5510` trên điện thoại,
   gọi `next_slide` (xem `docs/protocol.md`).
4. `Ctrl+S` → kiểm tra `%APPDATA%\ProSlides\Shows\*.show` và query FTS.
