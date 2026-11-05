#include <cmath>

class State {
public:
  float rotation[3] = {0, 0, 0};

  float previousAngularSpeed[3];
  float angularSpeed[3];

  float previousAcceleration[3];
  float acceleration[3];

  void update(float *a, float *angularSpeed, float deltaTime) {
    float accRotation[3];

    accRotation[0] = std::atan2(a[1], a[2]);
    accRotation[1] = std::atan2(-a[0], std::sqrt(a[1] * a[1] + a[2] * a[2]));
    accRotation[2] = 0;

    float gyroRotation[3];
    for (int i = 0; i < 3; i++)
      gyroRotation[i] = angularSpeed[i] * (3.1415f / 180.0f) * deltaTime;

    float alpha = 1.0f;

    for (int i = 0; i < 3; i++)
      rotation[i] = alpha * (rotation[i] + gyroRotation[i]);
  }

private:
};
