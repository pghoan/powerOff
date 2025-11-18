// File: AverageOverTime.h
#ifndef AverageOverTime_h
#define AverageOverTime_h

#include "Arduino.h"

class AverageOverTime {
private:
  unsigned long _durationSec;     // Thời gian lấy trung bình (giây)
  unsigned long _startTime;       // millis() khi bắt đầu
  float _sum;                     // Tổng các giá trị
  unsigned long _count;           // Số mẫu
  bool _finished;                 // Đã đủ thời gian chưa
  float _lastAverage;             // Lưu giá trị trung bình khi hoàn thành

public:
  // Constructor: chỉ cần thời gian (giây)
  AverageOverTime(unsigned long durationSec);

  // Bắt đầu một chu kỳ mới
  void begin();

  // Thêm một giá trị số thực bất kỳ lúc nào
  void addValue(float value);

  // Kiểm tra đã đủ thời gian chưa
  bool isFinished() const;

  // Lấy giá trị trung bình (chỉ có hiệu lực khi isFinished() == true)
  // Nếu chưa xong → trả về 0.0 hoặc giá trị hiện tại tùy nhu cầu (xem biến thể bên dưới)
  float getAverage() const;

  // Lấy số mẫu đã thêm
  unsigned long getSampleCount() const;

  // Reset để dùng lại
  void reset();
};

#endif