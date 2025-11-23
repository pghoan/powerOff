#include "AverageOverTime.h"
#include "EmonLib.h"                   // Include Emon Library
#define I_MIN  100.0  // ma
// TIMEOUT tính bằng phút
const unsigned long TIMEOUT_MS = (unsigned long)15 * 60000UL;
AverageOverTime avg(10);   // Trung bình trong 10 giây
EnergyMonitor emon1;                   // Create an instance
int CT_pin = 5; //  CT sensor pin connected to A5 pin of Arduino

#include <avr/sleep.h>
#include <avr/power.h>

const byte BUTTON_PIN = 2;  // D2 = INT0
//const unsigned long AWAKE_TIME_MS = 15UL * 60UL * 1000UL; // 15 phút

volatile bool wokeByButton = false;
unsigned long startMillis = 0;

const int ledPin = 13;     // LED
bool ledState = false;     // trạng thái LED

// ISR gọi khi có ngắt từ nút
void wakeISR() {
  wokeByButton = true;  // chỉ cần flag, việc còn lại làm sau khi tỉnh
}

// Hàm đưa Arduino vào sleep, chỉ dậy khi bấm nút
void goToSleep() {
  wokeByButton = false;

  // Cấu hình ngắt ngoài trên D2 (INT0), cạnh FALLING (từ HIGH xuống LOW)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), wakeISR, FALLING);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  // Tắt các ngoại vi để tiết kiệm (optional nhưng nên dùng)
  power_adc_disable();
  power_spi_disable();
  power_twi_disable();
  power_timer1_disable();
  power_timer2_disable();
  // (Timer0 dùng cho millis(), để vậy cũng được, nhưng khi ngủ thì nó dừng)

  noInterrupts();
  // Xoá cờ ngắt INT0 nếu có
  EIFR = bit(INTF0);
  interrupts();

  // ------ Bắt đầu ngủ ------
  sleep_cpu();
  // ------ Tỉnh dậy ở đây (sau khi bấm nút) ------

  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));

  // Bật lại các ngoại vi (nếu cần dùng)
  power_all_enable();

  // Bắt đầu đo thời gian hoạt động sau khi thức dậy
  startMillis = millis();
}

void setup() {
  Serial.begin(9600);
  avg.begin();             // Bắt đầu đếm 10 giây
  //emon1.current(CT_pin, 32.59);             // Current: input pin, calibration.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState); // off led

  // Nếu cần debug:
  // Serial.begin(9600);
  // delay(1000);
  // Serial.println("Booting, going to sleep...");

  // Ngay khi bật nguồn sẽ ngủ luôn, chờ bấm nút để dậy
  //goToSleep();
  
}

void loop () {
  digitalWrite(ledPin, true); // off led 
  delay(300);
  digitalWrite(ledPin, false); // off led 
  delay(300);
}
void loop_bak() {
  static uint32_t last = 0;
  static uint32_t last_imin = 0;

  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  Serial.print(Irms*230.0);         // Apparent power
  Serial.print(" ");
  Serial.println(Irms);          // Irms
return;
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
      if (last_imin == 0) {
        last_imin =millis();
        return;
      }
      if (millis() - last_imin >= TIMEOUT_MS) { 
        goToSleep();  // sẽ chỉ dậy khi bấm nút lần nữa
      }
    } 
    else {
     last_imin = 0;
     if (ledState==false) {
     ledState = true;
     digitalWrite(ledPin, ledState); // bat ro le
     }
    }
    avg.begin();
  }
  // Để tiết kiệm thêm chút, tránh vòng lặp quay quá nhanh
  delay(50);
  
}
