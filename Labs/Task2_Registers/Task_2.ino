#include <Arduino.h>

uint8_t A, B;     // inputs
uint8_t R;        // result register
uint8_t S;        // SREG copy

void setup() {
  Serial.begin(9600);
}

void loop() {

  if (!Serial.available())
    return;

  String input = Serial.readStringUntil('\n');

  int a, b;
  if (sscanf(input.c_str(), "%d %d", &a, &b) != 2)
    return;

  A = (uint8_t)a;
  B = (uint8_t)b;

  asm volatile(
    "mov r20, %[a]      \n\t"   // r20 = A
    "mov r21, %[b]      \n\t"   // r21 = B
    "sub r20, r21       \n\t"   // r20 = r20 - r21
    "in  r22, __SREG__  \n\t"   // copy SREG
    "mov %[res], r20    \n\t"   // store result
    "mov %[sreg], r22   \n\t"   // store SREG
    : [res] "=r"(R), [sreg] "=r"(S)
    : [a] "r"(A), [b] "r"(B)
    : "r20", "r21", "r22"
  );

  Serial.print("A=");
  Serial.print(A);

  Serial.print(" B=");
  Serial.print(B);

  Serial.print(" Result=");
  Serial.print(R);

  Serial.print(" Z=");
  Serial.print((S >> 1) & 1);

  Serial.print(" C=");
  Serial.println(S & 1);
}