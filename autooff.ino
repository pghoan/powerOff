#include "AverageOverTime.h"
#define I_MIN  100.0  // ma
#define TIMEOUT 15 // minutes
AverageOverTime avg(10);   // Trung bình trong 10 giây

void setup() {
  Serial.begin(115200);
  avg.begin();             // Bắt đầu đếm 10 giây
}

void loop() {
  static uint32_t last = 0;
  static uint32_t last_imin = 0;
  if (millis() - last >= 2) {   // Lấy mẫu mỗi 2ms (tùy ý)
    last = millis();

    float value = /* giá trị của bạn ở đây, ví dụ: */
                  analogRead(A0) * (5.0 / 1023.0);
                  // hoặc đọc từ BME280, DHT, tính toán, v.v.

    avg.addValue(value);
  }

  // Khi đủ 10 giây → lấy kết quả và xử lý
  if (avg.isFinished()) {
    float ket_qua = avg.getAverage();
    if (ket_qua < I_MIN) {
      if ((millis() -last_imin)/1000/60 > TIMEOUT) { // can than ham nay tinh sai
        // power off here
      }
    } 
    else {
     last_imin = millis();
    }
    avg.begin();
  }
}
