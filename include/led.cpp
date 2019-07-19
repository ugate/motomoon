
// ----------------- Flags -----------------------------

bool isBrakeOn() {
  return led_flags & BRAKE_ON && !(led_flags & BRAKE_OFF);
}
bool isBrakeOff() {
  return led_flags & BRAKE_OFF && !(led_flags & BRAKE_ON);
}
bool isTurnLeftOn() {
  return led_flags & LEFT_ON && !(led_flags & LEFT_OFF);
}
bool isTurnLeftOff() {
  return led_flags & LEFT_OFF && !(led_flags & LEFT_ON);
}
bool isTurnRightOn() {
  return led_flags & RIGHT_ON && !(led_flags & RIGHT_OFF);
}
bool isTurnRightOff() {
  return led_flags & RIGHT_OFF && !(led_flags & RIGHT_ON);
}
bool brakeOn() {
  led_flags &= ~BRAKE_OFF; // remove
  led_flags |= BRAKE_ON; // add
  //SERIAL_OUT.printf("Brakes on... BRAKE_ON: %d, BRAKE_OFF: %d\n", led_flags & BRAKE_ON, led_flags & BRAKE_OFF);
  return isBrakeOn();
}
bool brakeOff(bool now = false) {
  if (now) led_flags &= ~BRAKE_ON; // remove
  led_flags |= BRAKE_OFF; // add
  //SERIAL_OUT.printf("Brakes off... BRAKE_ON: %d, BRAKE_OFF: %d\n", led_flags & BRAKE_ON, led_flags & BRAKE_OFF);
  return isBrakeOff();
}
bool turnLeftOn() {
  led_flags &= ~LEFT_OFF; // remove
  led_flags |= LEFT_ON; // add
  //SERIAL_OUT.printf("Left turn signal on... LEFT_ON: %d, LEFT_OFF: %d\n", led_flags & LEFT_ON, led_flags & LEFT_OFF);
  return isTurnLeftOn();
}
bool turnLeftOff(bool now = false) {
  if (now) led_flags &= ~LEFT_ON; // remove
  led_flags |= LEFT_OFF; // add
  //SERIAL_OUT.printf("Left turn signal off... LEFT_ON: %d, LEFT_OFF: %d\n", led_flags & LEFT_ON, led_flags & LEFT_OFF);
  return isTurnLeftOff();
}
bool turnRightOn() {
  led_flags &= ~RIGHT_OFF; // remove
  led_flags |= RIGHT_ON; // add
  //SERIAL_OUT.printf("Right turn signal on... RIGHT_ON: %d, RIGHT_OFF: %d\n", led_flags & RIGHT_ON, led_flags & RIGHT_OFF);
  return isTurnRightOn();
}
bool turnRightOff(bool now = false) {
  if (now) led_flags &= ~RIGHT_ON; // remove
  led_flags |= RIGHT_OFF; // add
  //SERIAL_OUT.printf("Right turn signal off... RIGHT_ON: %d, RIGHT_OFF: %d\n", led_flags & RIGHT_ON, led_flags & RIGHT_OFF);
  return isTurnRightOff();
}

// ----------------- Program -----------------------------

void LoopAnimUpdate(const AnimationParam& param) {
  // wait for this animation to complete, we are using it as a timer of sorts
  if (param.state == AnimationState_Completed) {
    // done, time to restart this position tracking animation/timer
    animations.RestartAnimation(param.index);

    // draw the next frame in the sprite
    spriteSheet.Blt(strip, 0, indexSprite);
    indexSprite = (indexSprite + 1) % myImageHeight; // increment and wrap
  }
}

bool updatePixelImage(const File& file) {
  // initialize the image with the file
  if (!pixImage.Begin(file)) false;
  pixImage.Blt()
}

void ledSetup() {
  strip.Begin();
  strip.Show();

  indexSprite = 0;

  // we use the index 0 animation to time how often we rotate all the pixels
  animations.StartAnimation(0, 60/* PixelCount */, LoopAnimUpdate);
  statusIndicator(SETUP_STAT_LED_COMPLETE);
}

void ledLoop() {
  // avoid using delay() to prevent LED timing disruptions
  animations.UpdateAnimations();
  strip.Show();
}