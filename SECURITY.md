# Chính sách bảo mật — ProSlides

ProSlides xử lý dữ liệu dịch vụ (show, media, script) và có máy chủ điều khiển từ
xa trên mạng LAN, nên các vấn đề bảo mật được ưu tiên.

## Phiên bản được hỗ trợ

| Phiên bản | Trạng thái |
|---|---|
| `main` (đang phát triển) | ✅ Được hỗ trợ |
| Bản phát hành chính thức | Chưa có |
| Các fork/bản cũ | ❌ Không được hỗ trợ |

Vì dự án chưa phát hành bản ổn định, vui lòng kiểm tra và báo cáo lỗi trên nhánh `main`.

## Báo cáo lỗ hổng

**Không mở issue công khai cho lỗ hổng bảo mật.** Hãy dùng cơ chế báo cáo riêng của GitHub:

**[Báo cáo lỗ hổng riêng tư](https://github.com/minhvuong097/Pro-slides/security/advisories/new)**

Nếu không dùng được link trên, liên hệ qua issue thường với nội dung tối thiểu
"vui lòng liên hệ riêng" — **không** đính kèm chi tiết khai thác tại đó.

Nên đưa vào báo cáo:

- Mô tả lỗ hổng và mức độ ảnh hưởng (VD: remote control không xác thực, path traversal khi import `.show`).
- Các bước tái hiện tối thiểu.
- Phiên bản/commit, hệ điều hành, cấu hình (remote có bật không, có password không).
- PoC nếu có (gửi qua kênh riêng).

## Phạm vi quan tâm

- Giao thức remote WebSocket (`src/remote/`) và UDP discovery.
- Đọc/ghi file `.show`, `.fsb`, backup/restore (path traversal, file corrupt).
- FFmpeg/Qt Multimedia decode của media không tin cậy.
- CRDT sync qua relay (`src/cloud/`).
- SQL injection trong `src/persistence/` (hiện dùng prepared statement).

## Thời gian phản hồi

| Giai đoạn | Dự kiến |
|---|---|
| Xác nhận đã nhận | 3 ngày làm việc |
| Đánh giá ban đầu | 7 ngày làm việc |
| Cập nhật tiến độ | Mỗi 7 ngày |
| Phát hành bản vá | Tùy mức nghiêm trọng, ưu tiên lỗ hổng có thể khai thác từ mạng |

## Nguyên tắc khi báo cáo

- Không truy cập, sửa hoặc xóa dữ liệu của người dùng khác trong quá trình kiểm tra.
- Không thực hiện tấn công làm gián đoạn dịch vụ công cộng.
- Cho dự án thời gian hợp lý để phát hành bản vá trước khi công khai (coordinated disclosure).

## Safe harbor

Báo cáo lỗ hổng theo đúng chính sách này được coi là hoạt động nghiên cứu bảo mật
thiện chí. Chúng tôi sẽ không truy đòi hoặc khởi kiện các báo cáo viên tuân thủ
các nguyên tắc trên.
