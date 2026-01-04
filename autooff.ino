#include <Bounce2.h>

// test do dong
// nho mac capacitor de triet nhieu!!
#include "AverageOverTime.h"
#include "EmonLib.h"                   // Include Emon Library
#define I_MIN  0.1  // Ampere  // chinh so nay
// TIMEOUT tính bằng phút
const unsigned long TIMEOUT_MS = (unsigned long)60 * 60000UL; // chinh so nay
AverageOverTime avg(10);   // Trung bình trong 10 giây
EnergyMonitor emon1;                   // Create an instance
int CT_pin = A2; //  CT sensor pin connected to A5 pin of Arduino

#include <avr/sleep.h>
#include <avr/power.h>
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// INSTANTIATE A Button OBJECT FROM THE Bounce2 NAMESPACE
Bounce2::Button button = Bounce2::Button();


const byte BUTTON_PIN = 2;  // D2 = INT0
//const unsigned long AWAKE_TIME_MS = 15UL * 60UL * 1000UL; // 15 phút

volatile bool wokeByButton = false;
unsigned long startMillis = 0;

const int ledPin = 13;     // LED
const int controlPin = 4;  // chinh so nay
bool ledState = true;     // trạng thái LED

// ISR gọi khi có ngắt từ nút
void wakeISR() {
  wokeByButton = true;  // chỉ cần flag, việc còn lại làm sau khi tỉnh
}

// Hàm đưa Arduino vào sleep, chỉ dậy khi bấm nút
void goToSleep() {
  ledState = false;
  digitalWrite(ledPin, ledState); // tat ro le
  digitalWrite(controlPin, ledState); // tat ro le
  delay(1000);
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
  // ← KHI THỨC DẬY SẼ CHẠY TIẾP DÒNG TIẾP THEO, KHÔNG NHẢY LẠI setup()
  ledState = true;
  digitalWrite(ledPin, ledState); // bat ro le
  digitalWrite(controlPin, ledState); // tat ro le
  delay(5000); // thoi gian bat it nhat 5s
}

void setup() {
  //analogReference(INTERNAL); //Đặt lại mức điện áp tối đa  là 1,1 V (nếu sử dụng vi điều khiển ATmega328 hoặc ATmega168)
  //analogReference(DEFAULT);
  Serial.begin(9600);
  avg.begin();             // Bắt đầu đếm 10 giây
  emon1.current(CT_pin, 0.125);            // Current: input pin, calibration.
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(controlPin, OUTPUT);
  //digitalWrite(ledPin, ledState); // off led
  // 1) IF YOUR BUTTON HAS AN INTERNAL PULL-UP
  button.attach( BUTTON_PIN ,  INPUT_PULLUP ); // USE INTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  button.interval(5);

  // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  button.setPressedState(LOW);

  // Nếu cần debug:
  // Serial.begin(9600);
  Serial.println("Booting, going to sleep...");
  // Ngay khi bật nguồn sẽ ngủ luôn, chờ bấm nút để dậy
  goToSleep();
}

void loop() {
  static uint32_t last = millis();
  static uint32_t last_imin = 0; // thoi diem gan nhat co dong < I_MIN
  //Serial.println(Irms);          // Irms
  //delay(2000);
  // doc gia tri cam bien
  if (millis() - last >= 500) {   // Lấy mẫu mỗi 2ms (tùy ý)
    last = millis();
    float Irms = emon1.calcIrms(1480);  // Calculate Irms only
    avg.addValue(Irms);
  }
  // UPDATE THE BUTTON
  // YOU MUST CALL THIS EVERY LOOP
  button.update();
  if (button.isPressed()) {
    Serial.println("Button pressed...");
    goToSleep();  // sẽ chỉ dậy khi bấm nút lần nữa
    last_imin = millis();
  }
  // Khi đủ 10 giây → lấy kết quả và xử lý
  if (avg.isFinished()) {
    float ket_qua = avg.getAverage();
    Serial.println(ket_qua);          // Irms
    if (ket_qua < I_MIN) {
      if (millis() - last_imin >= TIMEOUT_MS  ) { // timout or bam phim --> ngu
        goToSleep();  // sẽ chỉ dậy khi bấm nút lần nữa
        last_imin = millis();
      }
    }
    else { // co tai
      last_imin = millis();
    }
    avg.begin();
  }
  // Để tiết kiệm thêm chút, tránh vòng lặp quay quá nhanh
  delay(50);

}
/*
   test case:
   bat dien --> led tắt
   dong < nguong & > 1 phut  --> led tat
   dong > nguong --> led sáng
   Dang sleep, bấm nút --> led sáng
*/
