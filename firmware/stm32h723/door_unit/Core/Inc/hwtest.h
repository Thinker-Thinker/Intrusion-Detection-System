/* Door unit hardware (Rev B wiring): servo, LEDs, buzzer, PIR, exit button, door switch,
 * battery/charger voltages and the ESP32 UART link. Also a small test command set on the UART. */
#ifndef HWTEST_H
#define HWTEST_H
#include <stdint.h>
void hwtest_init(void);        /* call once after the MX_*_Init() calls */
void hwtest_poll(void);        /* call often from the main loop */
const char *hwtest_lcd_line(void);  /* short status line for the LCD */

void hw_uart_puts(const char *s);   /* line to the ESP32 */
void hw_servo_deg(int deg);
void hw_led(int green, int red);
void hw_beep(uint32_t ms);
void hw_alarm(uint32_t ms);
int  hw_pir(void);
int  hw_exit_pressed(void);
int  hw_door_open(void);
void hw_test_outputs(void);
#endif
