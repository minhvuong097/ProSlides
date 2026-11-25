# Giao thức điều khiển từ xa

ProSlides dùng WebSocket (mặc định `ws://<host>:5510`) với envelope giống FreeShow:

```json
{ "channel": "API", "data": { "api": "next_slide", "data": {}, "send": true }, "id": "client-id" }
```

## 1. Handshake

1. Server gửi `CONNECTION {name, version}`
2. Server gửi `PASSWORD {password: bool, dictionary: {}}`
3. Client gửi `ACCESS {password: "..."}` (hoặc chuỗi rỗng nếu không có password)
4. Server trả `ACCESS true` + `THEME_COLORS {colors}`; quá 10 client → `ERROR {key:"overLimit"}`

## 2. Channels

| Channel | Hướng | Ý nghĩa |
|---|---|---|
| `CONNECTION` | S→C | Thông tin server |
| `PASSWORD` | S↔C | Challenge password |
| `ACCESS` | S↔C | Xác thực |
| `ERROR` | S→C | `wrongPass`, `overLimit` |
| `SHOWS` | S→C | Danh sách show |
| `SHOW` | S→C | Show đang mở |
| `PROJECTS` | S→C | Danh sách project |
| `OUT` / `OUT_DATA` | S→C | Trạng thái output |
| `API` | C→S / S→C | Gọi action, có `send:true` thì trả kết quả |
| `CRDT` | ↔ | Operation đồng bộ |

## 3. API actions đã hiện thực

| Action | Payload | Kết quả |
|---|---|---|
| `next_slide` | `{}` | index mới |
| `previous_slide` | `{}` | index mới |
| `index_select_slide` | `{index:int}` | index |
| `clear_all` | `{}` | `true` |
| `restore_output` | `{}` | `true` |
| `clear_slide` / `clear_background` | `{}` | `true` |
| `get_output` | `{}` | `{index, slide, cleared}` |
| `get_cleared` | `{}` | bool |
| `get_shows` | `{}` | mảng show |
| `get_groups` | `{}` | mảng tên nhóm |
| `get_slide` | `{slideId}` | JSON slide |
| `start_show` | `{id}` | `true` |
| `change_layout` | `{layoutId}` | `true` |
| `get_show_layout` / `get_projects` | `{}` | mảng |
| `start_timeline` / `pause_timeline` / `stop_timeline` | `{}` | `true` |
| `set_timeline_time` | `{seconds:number}` | vị trí ms |
| `get_timeline` | `{}` | `{position, duration, playing, actions}` |
| `search_scripture` | `{query}` | mảng kết quả |
| `start_scripture` | `{reference}` | `true` (tạo slide) |
| `scripture_next` / `scripture_previous` | `{}` | `true` |
| `get_scripture` | `{}` | `{reference, text}` |
| `name_start_timer` | `{name, seconds}` | timer id |
| `id_pause_timer` / `pause_timers` | `{id?}` | `true` |
| `id_stop_timer` / `stop_timers` | `{id?}` | `true` |
| `get_timers` | `{}` | mảng timer |
| `id_select_overlay` / `toggle_overlay` | `{id}` | `true` |
| `clear_overlays` / `get_overlays` | `{}` | `true` / mảng |
| `start_scene` / `clear_scene` | `{id}` | `true` |
| `play_audio` / `pause_audio` / `stop_audio` | `{}` | `true` |
| `audio_seekto` | `{seconds}` | vị trí ms |
| `change_volume` / `mute` | `{volume}` / `{value}` | `true` |
| `get_audio_mixer` / `create_audio_channel` | `{}` / `{name,path}` | mảng / channel id |
| `play_audio_channel` / `stop_audio_channel` | `{id}` | bool |
| `set_channel_volume` / `toggle_channel_mute` / `toggle_channel_solo` | `{id,volume}` / `{id}` | bool |
| `get_audio` / `get_playlists` | `{}` | trạng thái / mảng track |
| `get_triggers` / `connect_midi` | `{}` / `{device}` | mảng / bool |
| `transpose_show_up` / `transpose_show_down` | `{}` | `true` |
| `get_key` | `{}` | `"C"`, `"G"`... |
| `start_output_recording` | `{path}` | `true` |
| `stop_output_recording` / `toggle_output_recording` | `{}` / `{path}` | `true` / bool |
| `start_stream` / `stop_stream` | `{url, hls}` / `{}` | bool / `true` |
| `start_ndi` / `stop_ndi` | `{name}` / `{}` | bool / `true` |
| `get_backups` / `create_backup` / `restore_backup` | `{}` / `{label}` / `{id}` | mảng / id / bool |
| `get_plans` / `fetch_plans` / `import_plan` | `{}` / `{from,to}` / `{id}` | mảng / `true` / bool |
| `create_project` / `delete_project` | `{name}` / `{id}` | project id / bool |
| `add_to_project` / `remove_project_item` | `{projectId, showId, name}` / `{projectId, index}` | bool |
| `cloud_status` | `{}` | `{connected}` |
| `toggle_stage` / `start_stage` / `close_stage` | `{screen}` | bool / `true` |
| `stage_status` | `{}` | `{visible}` |

Các action khác của FreeShow (`play_media`, `start_timer`, `audio_*`, …) có thể
thêm bằng một nhánh trong `PresentationController::onApiRequested()`.

## 4. Discovery

UDP broadcast tới `255.255.255.255:5514` mỗi 5 giây:

```json
{"app":"ProSlides","name":"ProSlides-PC","port":5510,"version":"0.9.0"}
```

## 5. Ví dụ (JavaScript trong trình duyệt)

```js
const ws = new WebSocket("ws://192.168.1.10:5510");
ws.onmessage = (e) => {
  const msg = JSON.parse(e.data);
  if (msg.channel === "PASSWORD") ws.send(JSON.stringify({
    channel: "ACCESS", data: { password: "" }
  }));
};
setTimeout(() => ws.send(JSON.stringify({
  channel: "API",
  data: { api: "next_slide", data: {}, send: true }
})), 1000);
```
