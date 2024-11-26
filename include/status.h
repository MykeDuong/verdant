#pragma once

class VerdantStatus {
public:
  typedef enum {
    SUCCESS = 0,
    INTERNAL_ERROR = 1,
  } StatusEnum;

  static void handleError(VerdantStatus::StatusEnum status);
private:
  static VerdantStatus::StatusEnum value;
};

