# Kiến trúc & Data Model

## 1. Ánh xạ từ FreeShow

| FreeShow (TypeScript) | ProSlides (C++20) | Ghi chú |
|---|---|---|
| `Show` | `ps::Show` | `slides/layouts/media` là `std::map<Id, …>` |
| `Slide` | `ps::Slide` | `items`, `notes`, `group`, `key`, `timeline` |
| `Item` (~45 field) | `ps::Item` | field lạ → `extra: QJsonObject` |
| `Line`, `text[]`, `Chords` | `Line`, `TextPart`, `Chord` | chord `pos` = offset ký tự |
| `Layout`, `SlideData` | `Layout`, `SlideData` | `SlideData.id` trỏ tới `Slide` |
| `TimelineAction` | `TimelineAction` | `time` = **ms**, `duration` = **giây** |
| `Project`, `ProjectShowRef` | `Project`, `ProjectShowRef` | service flow |
| `Media` | `MediaDescriptor` | path tương đối/tuyệt đối |
| `.show` = `[id, Show]` | `encodeShowFile` / `decodeShowFile` | có repair reader cho file hỏng |

`std::optional<T>` được dùng cho mọi field `T?` của TypeScript để phân biệt
"không có field" với `null` (quan trọng vì FreeShow lọc field theo `in`-operator).

## 2. Lưu trữ

```
%APPDATA%/ProSlides/
├── Shows/*.show           # authoritative JSON, tương thích FreeShow
├── Media/
├── Backups/
└── proslides.db           # SQLite projection
```

Schema (`src/persistence/schema.sql`) gồm:
- `shows`, `slides`, `layouts`, `layout_slides`, `media`
- `projects`, `project_items`, `folders`
- `timeline_actions`, `overlays`, `templates`
- `slides_fts` (FTS5 external-content) + 3 trigger INSERT/DELETE/UPDATE

FTS5 dùng `tokenize='unicode61 remove_diacritics 2'` nên `tôi/Yêu` → `toi/yeu`.
`ShowRepository::ftsQuery()` cũng normalize input theo cách tương tự và thêm
prefix `*`.

Mọi ghi vào DB đều nằm trong `DbTransaction` (RAII) + `PRAGMA journal_mode=WAL`.

## 3. CRDT

`ps::crdt::Document` lưu một cây JSON tùy ý:

- **Field** → LWW register. Winner = `(Lamport time, actorId)` lớn nhất.
- **Sequence** → RGA: phần tử có `parent` (phần tử đứng trước lúc insert).
  Khi apply, các sibling được sort theo OpId giảm dần → deterministic convergence.
- **Log op** → `operationsSince(vectorClock)` gửi cho peer; idempotent qua `m_seen`.

Message trên wire:

```json
{"type":"ops","actor":"uuid","ops":[{...}],"vector":{"uuid":42}}
```

`SyncSession` làm anti-entropy: `hello` → trả ops thiếu → nhận ops của peer.
Local op được broadcast ngay trong `onLocalOperation`.

## 4. Video

```
VideoEngine (interface)
├── MultimediaVideoEngine   QMediaPlayer + QVideoSink (fallback)
└── FFmpegVideoEngine       thread riêng, D3D11VA/DXVA2/VAAPI
                            av_hwframe_transfer_data → swscale → QVideoFrame
```

Trên Windows, `preferredDeviceType()` trả `AV_HWDEVICE_TYPE_D3D11VA`. Frame GPU
được transfer một lần rồi đẩy tới `QVideoSink`. Đường zero-copy D3D11 texture →
Qt RHI có thể cắm thêm trong `FFmpegDecodeWorker::toImage()` nếu cần.

`NdiOutput` là no-op khi không build với `PS_ENABLE_NDI`.

## 5. Đã bổ sung (v0.10)

- `src/timeline/TimelineEngine` — playback engine, trigger action theo ms, loop,
  editor QML `TimelinePanel`.
- `src/scripture/` — Bible `.fsb` [id, Bible], parse reference, tìm kiếm và
  index FTS5 `verses_fts`; tạo slide kinh thánh từ remote/UI.
- `src/timers/TimerManager` — countdown/clock/stopwatch, cảnh báo màu, API remote.
- `src/templates/TemplateLibrary` — `templates.json` FreeShow, apply/create template.
- `src/overlays/OverlayManager` — overlay/scene FreeShow, bật/tắt, chuyển scene.
- `src/audio/AudioPlayer` — playlist audio, QMediaPlayer/QAudioOutput, volume/mute.
- `src/triggers/TriggerManager` + `MidiInput` — MIDI note/CC (winmm) và key → API action.
- API remote mở rộng: timeline + scripture + timers + overlays (xem `docs/protocol.md`).

## 6. Roadmap tiếp theo

- Keyframe easing trực quan cho style timeline
- Content providers (Planning Center, Canva) như FreeShow
- MIDI/HID trigger, audio routing, recording
- Cloud sync qua CRDT relay server
