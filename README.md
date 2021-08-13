# 2021 WWFIRST Elegoo Competition

There are a few things that I haven't had time to figure out so they are not used or are done in a 
less-than-ideal way.

- Interrupts weren't working consistently for the wheel encoders so switched to polling at the last
  minute.
- Some contructors (objects) weren't working as expected so I switched to "init" functions.
- A lot of the automation code performed less quickly or accurately that manual driving because the 
  ultrasonic sensor wasn't very accurate or fast and the wheel encoders were pretty coarse.
- Never took the time to smooth out the drive acceleration/deceleration.