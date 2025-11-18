// File: AverageOverTime.cpp
#include "AverageOverTime.h"

AverageOverTime::AverageOverTime(unsigned long durationSec) {
  _durationSec = durationSec;
  reset();
}

void AverageOverTime::begin() {
  reset();
  _startTime = millis();
}

void AverageOverTime::addValue(float value) {
  if (_finished) return;

  unsigned long now = millis();

  // Nếu đã vượt quá thời gian → kết thúc và tính trung bình
  if (now - _startTime >= _durationSec * 1000UL) {
    _finished = true;
    if (_count > 0) {
      _lastAverage = _sum / _count;
    } else {
      _lastAverage = 0.0f;
    }
    return;
  }

  // Còn trong thời gian → cộng dồn bình thường
  _sum += value;
  _count++;
}

bool AverageOverTime::isFinished() const {
  return _finished;
}

float AverageOverTime::getAverage() const {
  if (_finished && _count > 0) {
    return _lastAverage;
  }
  return 0.0f;              // Chưa xong hoặc chưa có mẫu nào
}

unsigned long AverageOverTime::getSampleCount() const {
  return _count;
}

void AverageOverTime::reset() {
  _sum = 0.0f;
  _count = 0;
  _startTime = 0;
  _finished = false;
  _lastAverage = 0.0f;
}