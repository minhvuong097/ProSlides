# Đóng góp cho ProSlides

Cảm ơn bạn quan tâm đến dự án. ProSlides là phần mềm GPL-3.0; mọi đóng góp đều được hiểu là phát hành dưới cùng giấy phép.

## Bắt đầu

1. Fork repo, tạo branch từ `main`, ví dụ `feature/lyric-editor`.
2. Build theo [docs/build-windows.md](docs/build-windows.md). Tối thiểu cần Qt 6 (≥ 6.5).
3. Chạy test: `ctest --test-dir build -C Release --output-on-failure`.
4. Mở PR mô tả rõ thay đổi, cách kiểm tra, và ảnh hưởng tới dữ liệu `.show`.

## Quy ước code

- C++20, không dùng extension của compiler.
- Qt naming: class `CamelCase`, method `camelCase`, member `m_camelCase`.
- Giữ tương thích dữ liệu FreeShow: field TypeScript `?` → `std::optional<T>`;
  field lạ trong `Item` phải được giữ trong `Item::extra`.
- Không tự ý đổi unit timeline: `time` = ms, `duration` = giây.
- Mọi thao tác ghi DB phải nằm trong `DbTransaction`.
- Plugin/backend mới phải có interface và stub khi dependency không có.

## Test

Thêm test cho: serialization, CRDT convergence, query FTS5, và luồng UI quan trọng.
Chạy `ctest` trước khi gửi PR. CI chạy build + test trên Windows.

## Issue

Dùng template có sẵn:
- Bug report: steps to reproduce, phiên bản Qt, file `.show` tối thiểu (nếu liên quan).
- Feature request: mô tả UX, lý do, và phạm vi dữ liệu bị ảnh hưởng.

## License

Bằng cách gửi PR, bạn đồng ý rằng đóng góp của mình được cấp dưới GPL-3.0.

## Thương hiệu

Đừng đổi tên dự án hoặc logo trong PR trừ khi được yêu cầu. Quyền đối với tên và
logo là riêng biệt với GPL — xem [TRADEMARK.md](TRADEMARK.md).

## Bảo mật

Không gửi lỗ hổng bảo mật qua issue hoặc PR công khai. Xem [SECURITY.md](SECURITY.md)
để biết kênh báo cáo riêng.
