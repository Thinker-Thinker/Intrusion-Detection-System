/* Hardware test for the door unit (Rev B wiring): servo, LEDs, buzzer, LED strip,
 * PIR, exit button, door switch, battery/charger voltages and the ESP32 UART link.
 * Runs alongside the camera stream. */
#ifndef HWTEST_H
#define HWTEST_H

void hwtest_init(void);        /* call once after the MX_*_Init() calls */
void hwtest_poll(void);        /* call often from the main loop */
const char *hwtest_lcd_line(void);  /* short status line for the LCD */

#endif
