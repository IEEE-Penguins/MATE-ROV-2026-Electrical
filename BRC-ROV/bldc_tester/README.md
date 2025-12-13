# BLDC tester

### A device used for testing BLDC(Brushless DC) motors by sending a pwm signal between 1100ms and 1900ms.
<br>

```1100ms -> max counter-clock wise direction```
<br>

```1500ms -> stop```
<br>

```1900ms -> max clock wise direction```
<br>

* The device sends this signal via pin 3 which should be connected with the signal pin of ESC in addition to the ground and vcc(is needed).
<br>

* <b style="color:red">⚠️⚠️⚠️Note: check if the ECS is 5v logic tolerant or not!</b>
