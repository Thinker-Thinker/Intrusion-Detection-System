/* Hardware test for the door unit (Rev B wiring). Register-level for USART1, TIM3 and ADC1
 * (their HAL drivers are not in this project); HAL only for GPIO and clocks.
 *
 *  PA9  USART1_TX -> ESP32 GPIO16      PA10 USART1_RX <- ESP32 GPIO17   (LINK_BAUD 8N1)
 *  PD8  ESP32 EN (open-drain, released)
 *  PB0  TIM3_CH3 servo PWM, 50 Hz      PD14 buzzer (high = on)
 *  PA0  LED strip MOSFET (high = on)   PD0 green LED, PD1 red LED (high = on)
 *  PD10 PIR out (high = motion)        PD4 exit button, PD5 door reed (to GND, pull-up)
 *  PC0  ADC1_INP10 charger sense (100k/150k)   PC1 ADC1_INP11 battery sense (100k/27k)
 *
 * Commands (one per line from the ESP32): PING, STATUS, TEST, SERVO <deg>, SWEEP, STOP,
 * BEEP, LED G|R|OFF, ESPRESET. Status lines start with "HW ". Any other line goes to door_command()
 * (ENROL, LIST, OPEN, ...), so the ESP32 can drive the door logic too.
 * The LED strip on PA0 was dropped from the design; PA0 is just held low.
 */
#include "main.h"
#include "hwtest.h"
#include "door.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SENSE_MAINS_RATIO ((100.0f + 150.0f) / 150.0f)   /* R8/R9   */
#define SENSE_VSYS_RATIO  ((100.0f + 27.0f) / 27.0f)     /* R10/R11 */
#define VREF 3.3f

/* ------------------------------------------------------------------ UART1 */
#define LINK_BAUD 921600u          /* fast enough for a 38 KB unknown-face record in about 0.4 s */
#define RXN 4096                   /* holds an admin command (up to ~760 bytes) while a frame is processed */
static volatile uint8_t rxbuf[RXN];
static volatile uint16_t rx_head, rx_tail;

void USART1_IRQHandler(void)
{
  uint32_t isr = USART1->ISR;
  if (isr & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE))
    USART1->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF;
  while (USART1->ISR & USART_ISR_RXNE_RXFNE)
  {
    uint8_t c = (uint8_t)USART1->RDR;
    uint16_t n = (rx_head + 1) & (RXN - 1);
    if (n != rx_tail) { rxbuf[rx_head] = c; rx_head = n; }
  }
}

static void uart_init(void)
{
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  g.Pin = GPIO_PIN_9 | GPIO_PIN_10; g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_LOW; g.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &g);
  USART1->CR1 = 0;
  USART1->BRR = (HAL_RCC_GetPCLK2Freq() + LINK_BAUD / 2) / LINK_BAUD;   /* kernel clock = PCLK2 (reset default) */
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE | USART_CR1_UE;
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void hw_uart_write(const uint8_t *p, uint32_t n)
{
  while (n--)
  {
    uint32_t t0 = HAL_GetTick();
    while (!(USART1->ISR & USART_ISR_TXE_TXFNF)) if (HAL_GetTick() - t0 > 5) return;
    USART1->TDR = *p++;
  }
}

void hw_uart_puts(const char *s)
{
  while (*s)
  {
    uint32_t t0 = HAL_GetTick();
    while (!(USART1->ISR & USART_ISR_TXE_TXFNF)) if (HAL_GetTick() - t0 > 5) return;
    USART1->TDR = (uint8_t)*s++;
  }
}

/* ------------------------------------------------------------------ servo */
static int servo_us = 1500, sweeping, sweep_dir = 20;

static void servo_init(void)
{
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();
  g.Pin = GPIO_PIN_0; g.Mode = GPIO_MODE_AF_PP; g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW; g.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOB, &g);
  uint32_t timclk = HAL_RCC_GetPCLK1Freq();
  if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >= RCC_D2CFGR_D2PPRE1_2) timclk *= 2;  /* APB1 divided: timers x2 */
  TIM3->CR1 = 0;
  TIM3->PSC = timclk / 1000000 - 1;            /* 1 us ticks */
  TIM3->ARR = 20000 - 1;                       /* 20 ms = 50 Hz */
  TIM3->CCR3 = servo_us;
  TIM3->CCMR2 = (6 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;   /* PWM mode 1 */
  TIM3->CCER = TIM_CCER_CC3E;
  TIM3->EGR = TIM_EGR_UG;
  TIM3->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}

static void servo_set_us(int us)
{
  if (us < 500) us = 500;
  if (us > 2500) us = 2500;
  servo_us = us; TIM3->CCR3 = us;
}

/* ------------------------------------------------------------------ ADC1 */
static int adc_ok;

static void adc_init(void)
{
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_0 | GPIO_PIN_1; g.Mode = GPIO_MODE_ANALOG; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &g);

  RCC->CR |= RCC_CR_HSION;                                  /* per_ck = HSI 64 MHz */
  while (!(RCC->CR & RCC_CR_HSIRDY)) {}
  RCC->D1CCIPR &= ~RCC_D1CCIPR_CKPERSEL;                    /* CKPER = HSI */
  RCC->D3CCIPR = (RCC->D3CCIPR & ~RCC_D3CCIPR_ADCSEL) | (2 << RCC_D3CCIPR_ADCSEL_Pos);   /* ADC clock = per_ck */
  __HAL_RCC_ADC12_CLK_ENABLE();
  ADC12_COMMON->CCR = (ADC12_COMMON->CCR & ~(ADC_CCR_CKMODE | ADC_CCR_PRESC)) | (1 << ADC_CCR_PRESC_Pos); /* /2 = 32 MHz */

  ADC1->CR &= ~ADC_CR_DEEPPWD;
  ADC1->CR |= ADC_CR_ADVREGEN;
  HAL_Delay(2);
  ADC1->CR = (ADC1->CR & ~ADC_CR_BOOST) | (3 << ADC_CR_BOOST_Pos);   /* 25-50 MHz range */
  ADC1->CR &= ~ADC_CR_ADCALDIF;
  ADC1->CR |= ADC_CR_ADCALLIN;
  ADC1->CR |= ADC_CR_ADCAL;
  uint32_t t0 = HAL_GetTick();
  while (ADC1->CR & ADC_CR_ADCAL) if (HAL_GetTick() - t0 > 50) return;

  ADC1->PCSEL_RES0 |= (1 << 10) | (1 << 11);
  ADC1->SMPR2 = (ADC1->SMPR2 & ~0x3Fu) | (7 << 0) | (7 << 3);        /* longest sampling: 100k sources */
  ADC1->CFGR = (ADC1->CFGR & ~ADC_CFGR_RES) | ADC_CFGR_OVRMOD;       /* 16-bit, single conversions */
  ADC1->ISR = ADC_ISR_ADRDY;
  ADC1->CR |= ADC_CR_ADEN;
  t0 = HAL_GetTick();
  while (!(ADC1->ISR & ADC_ISR_ADRDY)) if (HAL_GetTick() - t0 > 50) return;
  adc_ok = 1;
}

static float adc_volts(int ch)
{
  if (!adc_ok) return -1.0f;
  uint32_t sum = 0;
  for (int i = 0; i < 8; i++)
  {
    ADC1->SQR1 = (uint32_t)ch << ADC_SQR1_SQ1_Pos;   /* one conversion, channel ch */
    ADC1->CR |= ADC_CR_ADSTART;
    uint32_t t0 = HAL_GetTick();
    while (!(ADC1->ISR & ADC_ISR_EOC)) if (HAL_GetTick() - t0 > 5) return -1.0f;
    sum += ADC1->DR;
  }
  return (sum / 8.0f) * VREF / 65535.0f;
}

/* ------------------------------------------------------------------ GPIO */
static void gpio_init(void)
{
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  g.Mode = GPIO_MODE_OUTPUT_PP; g.Pull = GPIO_NOPULL; g.Speed = GPIO_SPEED_FREQ_LOW;
  g.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14; HAL_GPIO_Init(GPIOD, &g);
  g.Pin = GPIO_PIN_0; HAL_GPIO_Init(GPIOA, &g);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);      /* ESP32 EN released */
  g.Mode = GPIO_MODE_OUTPUT_OD; g.Pin = GPIO_PIN_8; HAL_GPIO_Init(GPIOD, &g);

  g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_PULLUP; g.Pin = GPIO_PIN_4 | GPIO_PIN_5; HAL_GPIO_Init(GPIOD, &g);
  g.Pull = GPIO_PULLDOWN; g.Pin = GPIO_PIN_10; HAL_GPIO_Init(GPIOD, &g);
}

#define LED_G(on)   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, (on) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define LED_R(on)   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, (on) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define BUZZ(on)    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, (on) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define STRIP(on)   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (on) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define PIR()       (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) == GPIO_PIN_SET)
#define BTN()       (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4) == GPIO_PIN_RESET)   /* pressed = 1 */
#define DOOR_OPEN() (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == GPIO_PIN_SET)     /* reed open = 1 */

/* ------------------------------------------------------------------ test sequence */
static int test_step = -1;       /* -1 = idle */
static uint32_t test_t, beep_until, alarm_until;
static void beep(uint32_t ms) { BUZZ(1); beep_until = HAL_GetTick() + ms; }

/* ------------------------------------------------------------------ API for the door logic */
void hw_servo_deg(int deg)
{
  if (deg < 0) deg = 0;
  if (deg > 180) deg = 180;
  sweeping = 0; servo_set_us(500 + deg * 2000 / 180);
}
void hw_led(int green, int red) { LED_G(green); LED_R(red); }
void hw_beep(uint32_t ms) { alarm_until = 0; beep(ms); }
void hw_alarm(uint32_t ms) { alarm_until = HAL_GetTick() + ms; }    /* 4 Hz on/off buzzer */
int hw_pir(void) { return PIR(); }
int hw_exit_pressed(void) { return BTN(); }
int hw_door_open(void) { return DOOR_OPEN(); }
void hw_test_outputs(void) { test_step = 0; test_t = 0; }   /* LEDs, buzzer, servo, one after another */

static void test_run(void)      /* non-blocking: one step every 600 ms */
{
  if (test_step < 0 || HAL_GetTick() - test_t < 600) return;
  test_t = HAL_GetTick();
  switch (test_step++)
  {
  case 0: hw_uart_puts("HW test: green LED\n"); LED_G(1); LED_R(0); break;
  case 1: hw_uart_puts("HW test: red LED\n"); LED_G(0); LED_R(1); break;
  case 2: hw_uart_puts("HW test: buzzer\n"); LED_R(0); beep(200); break;
  case 3: hw_uart_puts("HW test: servo 0 deg\n"); sweeping = 0; servo_set_us(500); break;
  case 4: hw_uart_puts("HW test: servo 90 deg\n"); servo_set_us(1500); break;
  case 5: hw_uart_puts("HW test: servo 0 deg (door closed)\n"); servo_set_us(500); break;
  default: hw_uart_puts("HW test: done\n"); test_step = -1; break;
  }
}

/* ------------------------------------------------------------------ status */
static char lcd_line[40];

static void status(int force)
{
  static int last_pir = -1, last_btn = -1, last_door = -1;
  static uint32_t last_t;
  int pir = PIR(), btn = BTN(), door = DOOR_OPEN();
  int changed = pir != last_pir || btn != last_btn || door != last_door;
  if (!force && !changed && HAL_GetTick() - last_t < 2000) return;
  last_pir = pir; last_btn = btn; last_door = door; last_t = HAL_GetTick();

  float mains = adc_volts(10) * SENSE_MAINS_RATIO, vsys = adc_volts(11) * SENSE_VSYS_RATIO;
  char s[160];
  if (adc_ok)
    snprintf(s, sizeof s, "HW pir=%d exit_btn=%d door_open=%d battery=%.2fV charger=%.2fV servo=%dus\n",
             pir, btn, door, vsys, mains, servo_us);
  else
    snprintf(s, sizeof s, "HW pir=%d exit_btn=%d door_open=%d battery=? (ADC not ready) servo=%dus\n",
             pir, btn, door, servo_us);
  hw_uart_puts(s);
  snprintf(lcd_line, sizeof lcd_line, "P%d B%d D%d %.1fV", pir, btn, door, adc_ok ? vsys : 0.0f);
}

const char *hwtest_lcd_line(void) { return lcd_line; }

/* ------------------------------------------------------------------ commands */
static void handle(char *line)
{
  char cmd[48];
  int i = 0;
  for (; line[i] && i < (int)sizeof cmd - 1; i++) cmd[i] = (line[i] >= 'a' && line[i] <= 'z') ? line[i] - 32 : line[i];
  cmd[i] = 0;
  char out[96];
  if (!strcmp(cmd, "PING"))
  {
    snprintf(out, sizeof out, "PONG stm32h723 up=%lus\n", (unsigned long)(HAL_GetTick() / 1000));
    hw_uart_puts(out);
  }
  else if (!strcmp(cmd, "STATUS")) status(1);
  else if (!strcmp(cmd, "TEST")) { test_step = 0; test_t = 0; }
  else if (!strncmp(cmd, "SERVO", 5))
  {
    int deg = atoi(cmd + 5);
    if (deg < 0) deg = 0;
    if (deg > 180) deg = 180;
    sweeping = 0; servo_set_us(500 + deg * 2000 / 180);
    snprintf(out, sizeof out, "OK servo %d deg (%d us)\n", deg, servo_us); hw_uart_puts(out);
  }
  else if (!strcmp(cmd, "SWEEP")) { sweeping = 1; hw_uart_puts("OK sweeping\n"); }
  else if (!strcmp(cmd, "STOP")) { sweeping = 0; servo_set_us(1500); hw_uart_puts("OK servo centre\n"); }
  else if (!strcmp(cmd, "BEEP")) { beep(300); hw_uart_puts("OK beep\n"); }
  else if (!strcmp(cmd, "LED G")) { LED_G(1); LED_R(0); hw_uart_puts("OK green\n"); }
  else if (!strcmp(cmd, "LED R")) { LED_G(0); LED_R(1); hw_uart_puts("OK red\n"); }
  else if (!strcmp(cmd, "LED OFF")) { LED_G(0); LED_R(0); hw_uart_puts("OK LEDs off\n"); }
  else if (!strcmp(cmd, "ESPRESET"))
  {
    hw_uart_puts("OK resetting ESP32 via EN (PD8)\n");
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET); HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
  }
  else if (cmd[0])
    door_command(line, DOOR_SRC_UART);
}

/* ------------------------------------------------------------------ public */
void hwtest_init(void)
{
  gpio_init();
  servo_init();
  uart_init();
  adc_init();
  servo_set_us(500);                       /* door closed */
  hw_uart_puts("\nHW STM32 door unit ready. Send PING, STATUS, TEST or HELP.\n");
  status(1);
}

void hwtest_poll(void)
{
  static char line[1024];
  static int n;
  while (rx_tail != rx_head)
  {
    char c = (char)rxbuf[rx_tail]; rx_tail = (rx_tail + 1) & (RXN - 1);
    if (c == '\r') continue;
    if (c == '\n') { line[n] = 0; handle(line); n = 0; }
    else if (n < (int)sizeof line - 1) line[n++] = c;
  }

  if (beep_until && (int32_t)(HAL_GetTick() - beep_until) >= 0) { BUZZ(0); beep_until = 0; }
  if (alarm_until)
  {
    if ((int32_t)(HAL_GetTick() - alarm_until) >= 0) { BUZZ(0); alarm_until = 0; }
    else BUZZ((HAL_GetTick() / 125) & 1);
  }

  static uint32_t sweep_t;
  if (sweeping && HAL_GetTick() - sweep_t >= 20)
  {
    sweep_t = HAL_GetTick();
    if (servo_us + sweep_dir > 2500 || servo_us + sweep_dir < 500) sweep_dir = -sweep_dir;
    servo_set_us(servo_us + sweep_dir);
  }

  test_run();
  status(0);
}
