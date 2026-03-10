// ================================================================
//  WallReadings.h — Struct for wall sensor data
//
//  Placed in a separate header to avoid the Arduino IDE's
//  auto-generated function prototype ordering issue, which causes
//  "does not name a type" errors when structs are defined in .ino
//  files and used as function return types.
// ================================================================

#ifndef WALL_READINGS_H
#define WALL_READINGS_H

struct WallReadings {
  // Raw 10-bit ADC values (0–1023). Higher = closer.
  int leftRaw;
  int frontRaw;
  int rightRaw;

  // Boolean wall presence (raw > WALL_DETECT_THRESHOLD)
  bool leftWall;
  bool frontWall;
  bool rightWall;

  // Close proximity (raw > WALL_CLOSE_THRESHOLD)
  bool leftClose;
  bool frontClose;
  bool rightClose;

  // Danger zone (raw > WALL_TOO_CLOSE)
  bool leftDanger;
  bool frontDanger;
  bool rightDanger;
};

#endif // WALL_READINGS_H
