#include "AverageOverTime.h"
#include "EmonLib.h"                   // Include Emon Library
#define I_MIN  100.0  // ma
#define TIMEOUT 15 // minutes
AverageOverTime avg(10);   // Trung bình trong 10 giây
EnergyMonitor emon1;                   // Create an instance
int CT_pin = 5; //  CT sensor pin connected to A5 pin of Arduino

void setup() {
  Serial.begin(9600);
  avg.begin();             // Bắt đầu đếm 10 giây
  emon1.current(CT_pin, 32.59);             // Current: input pin, calibration.
}

void loop() {
  static uint32_t last = 0;
  static uint32_t last_imin = 0;

  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  
  Serial.print(Irms*230.0);         // Apparent power
  Serial.print(" ");
  Serial.println(Irms);          // Irms

  // doc gia tri cam bien
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
