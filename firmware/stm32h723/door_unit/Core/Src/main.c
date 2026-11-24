/* USER CODE BEGIN Header */

/*---------------------------------------
- WeAct Studio Official Link
- taobao: weactstudio.taobao.com
- aliexpress: weactstudio.aliexpress.com
- github: github.com/WeActStudio
- gitee: gitee.com/WeActStudio
- blog: blog.weactstudio.com
---------------------------------------*/

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board.h"
#include "lcd.h"
#include "camera.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hwtest.h"
#include "extflash.h"
#include "face_models.h"
#include "door.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Capture at QVGA (320x240) RGB565: the working resolution for every model.
 * The frame lives at the start of the shared 320 KB AXI SRAM pool (face_models.c): the models reuse
 * that RAM once the face crops are made, so each frame is used exactly once, then the next is captured. */
#define FRAME_W 320
#define FRAME_H 240
#define FRAME_BYTES (FRAME_W * FRAME_H * 2)
#define frame FM_FRAME

/* 0.96" ST7735 panel: 160x80 landscape. Preview = 2x downscale, centre crop (frame rows 40..199). */
#define PREVIEW_W 160
#define PREVIEW_H 80
#define PREVIEW_Y0 20

volatile uint32_t DCMI_FrameIsReady = 0;
uint32_t Camera_FPS = 0;

/* Frame header sent before a frame on the USB virtual COM port (SNAP / STREAM 1).
 * Little-endian, 16 bytes. tools/door.py and tools/capture.py parse it. */
typedef struct __attribute__((packed)) {
  char     magic[4];   /* "FRM2" */
  uint16_t width;
  uint16_t height;
  uint16_t format;     /* 1 = RGB565, bytes in sensor order (high byte first) */
  uint16_t seq;
  uint32_t length;     /* payload bytes that follow */
  uint32_t crc32;      /* CRC-32 (zlib/IEEE) of the payload, lets the PC drop damaged frames */
} frame_header_t;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void start_capture(void)
{
  DCMI_FrameIsReady = 0;
  /* The models leave dirty cache lines over the frame area: write them back now, or they could be
   * evicted on top of the new frame while the DMA fills it. */
  SCB_CleanInvalidateDCache_by_Addr((uint32_t *)frame, FRAME_BYTES);
  /* The DCMI DMA stream is circular, so it is still running after a snapshot.
   * Stop (aborts the DMA) before arming the next capture. */
  HAL_DCMI_Stop(&hdcmi);
  /* Snapshot mode: DCMI captures exactly one frame, then stops. Length is in
   * 32-bit words. */
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame, FRAME_BYTES / 4);
}

/* 2x downscale of the QVGA frame into the 160x80 preview (rows 20..99 of the
 * 160x120 downscaled image), then push it to the LCD. */
static void show_preview(void)
{
  uint16_t *preview = FM_PREVIEW;        /* scratch in the pool, free until the detector runs */
  const uint16_t *src = (const uint16_t *)frame;
  for (int y = 0; y < PREVIEW_H; y++)
  {
    const uint16_t *row = &src[(2 * (y + PREVIEW_Y0)) * FRAME_W];
    for (int x = 0; x < PREVIEW_W; x++)
      preview[y * PREVIEW_W + x] = row[2 * x];
  }
  ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)preview, PREVIEW_W, PREVIEW_H);
}

/* ---- Exposure target (OV7725 AEC/AGC stable region) -------------------
 * AEW/AEB = upper/lower brightness limits the sensor's auto-exposure aims
 * between; VPT = the band outside which it adjusts quickly. Raising them
 * makes the picture brighter, lowering them darker. '[' / ']' from the PC. */
#define OV7725_GAIN 0x00
#define OV7725_AEW  0x24
#define OV7725_AEB  0x25
#define OV7725_VPT  0x26
static uint8_t ae_target;      /* = AEW; AEB kept 0x10 below it */

static uint8_t cam_rd(uint8_t reg) { uint8_t v = 0; Camera_ReadReg(&hcamera, reg, &v); return v; }
static void cam_wr(uint8_t reg, uint8_t v) { Camera_WriteReg(&hcamera, reg, &v); }

static void set_ae_target(int t)
{
  if (t < 0x20) t = 0x20;
  if (t > 0xE0) t = 0xE0;
  uint8_t aew = (uint8_t)t, aeb = (uint8_t)(t - 0x10);
  uint8_t hi = (uint8_t)(((aew >> 4) + 1) > 0xF ? 0xF : ((aew >> 4) + 1));
  uint8_t lo = (uint8_t)((aeb >> 4) > 0 ? (aeb >> 4) - 1 : 0);
  cam_wr(OV7725_AEW, aew);
  cam_wr(OV7725_AEB, aeb);
  cam_wr(OV7725_VPT, (uint8_t)((hi << 4) | lo));
  ae_target = aew;
}

/* ---- Face-weighted auto-brightness --------------------------------------
 * The sensor's own auto-exposure meters the whole frame, so a bright doorway
 * or sky behind a person darkens their face. This loop measures only the
 * centre of the frame (where the face is) and nudges the sensor's
 * brightness target until the centre sits near AUTO_LUMA_TARGET.
 * Later: measure the detected face box instead of the fixed centre, and when
 * the target is at its maximum and gain stays high, raise the LED (PWM). */
#define AUTO_LUMA_TARGET   120   /* desired mean luma of the centre, 0-255 */
#define AUTO_LUMA_DEADBAND  12   /* no change while within +/- this */
#define AUTO_PERIOD_MS     600   /* one step at most this often (no flicker) */
#define AUTO_STEP         0x08
#define AE_MIN            0x30
#define AE_MAX            0xD8
static uint8_t auto_bright = 1;  /* 'A' from the PC toggles; [ / ] turn it off */
static uint8_t centre_luma;
static uint32_t last_auto_tick;

/* Mean luma (BT.601, 0-255) of the centre 50% x 60% of the frame,
 * sampling every 4th pixel and every 4th row. Pixels are RGB565, high byte first. */
static uint8_t measure_centre_luma(void)
{
  uint32_t sum = 0, n = 0;
  for (int y = FRAME_H / 5; y < FRAME_H * 4 / 5; y += 4)
  {
    const uint8_t *row = &frame[(y * FRAME_W) * 2];
    for (int x = FRAME_W / 4; x < FRAME_W * 3 / 4; x += 4)
    {
      uint16_t px = (uint16_t)((row[2 * x] << 8) | row[2 * x + 1]);
      uint32_t r = (px >> 8) & 0xF8, g = (px >> 3) & 0xFC, b = (px << 3) & 0xF8;
      sum += (77 * r + 150 * g + 29 * b) >> 8;
      n++;
    }
  }
  return (uint8_t)(n ? sum / n : 0);
}

static void auto_brightness_step(void)
{
  centre_luma = measure_centre_luma();
  if (!auto_bright || HAL_GetTick() - last_auto_tick < AUTO_PERIOD_MS) return;
  last_auto_tick = HAL_GetTick();
  int t = ae_target;
  if (centre_luma < AUTO_LUMA_TARGET - AUTO_LUMA_DEADBAND)      t += AUTO_STEP;
  else if (centre_luma > AUTO_LUMA_TARGET + AUTO_LUMA_DEADBAND) t -= AUTO_STEP;
  else return;
  if (t < AE_MIN) t = AE_MIN;
  if (t > AE_MAX) t = AE_MAX;
  if (t != ae_target) set_ae_target(t);
  /* TODO (LED): if t == AE_MAX and gain stays high, increase LED PWM here. */
}

/* Camera commands, called by door_command() with the upper-cased line. */
int cam_command(const char *u, char *out, int n)
{
  if (!strcmp(u, "AE +")) { auto_bright = 0; set_ae_target(ae_target + 0x08); }
  else if (!strcmp(u, "AE -")) { auto_bright = 0; set_ae_target(ae_target - 0x08); }
  else if (!strncmp(u, "AUTO", 4)) auto_bright = (u[4] == ' ') ? (u[5] != '0') : !auto_bright;
  else if (strcmp(u, "INFO") && strcmp(u, "CAM")) return 0;
  snprintf(out, n, "CAM id=0x%04x w=%d h=%d fps=%lu ae=0x%02x gain=0x%02x auto=%d luma=%d\n",
           hcamera.device_id, FRAME_W, FRAME_H, (unsigned long)Camera_FPS,
           ae_target, cam_rd(OV7725_GAIN), auto_bright, centre_luma);
  return 1;
}

/* Face box and status text over the preview. */
static void lcd_hline(int x, int y, int w, uint16_t c)
{
  if (y < 0 || y >= PREVIEW_H) return;
  if (x < 0) { w += x; x = 0; }
  if (x + w > PREVIEW_W) w = PREVIEW_W - x;
  if (w > 0) ST7735_LCD_Driver.FillRect(&st7735_pObj, x, y, w, 1, c);
}
static void lcd_vline(int x, int y, int h, uint16_t c)
{
  if (x < 0 || x >= PREVIEW_W) return;
  if (y < 0) { h += y; y = 0; }
  if (y + h > PREVIEW_H) h = PREVIEW_H - y;
  if (h > 0) ST7735_LCD_Driver.FillRect(&st7735_pObj, x, y, 1, h, c);
}
static void draw_overlay(void)
{
  int x, y, w, h; uint16_t c;
  door_face_box(&x, &y, &w, &h, &c);
  if (w > 0) {
    x /= 2; y = y / 2 - PREVIEW_Y0; w /= 2; h /= 2;
    lcd_hline(x, y, w, c); lcd_hline(x, y + h, w, c);
    lcd_vline(x, y, h, c); lcd_vline(x + w, y, h, c);
  }
  char msg[28];
  snprintf(msg, sizeof msg, "%-26s", door_lcd_text(&c));   /* pad: clears the rest of the line */
  uint16_t old = POINT_COLOR; POINT_COLOR = c;
  LCD_ShowString(2, 66, 156, 12, 12, (uint8_t *)msg);
  POINT_COLOR = old;
}

static int send_frame(uint16_t seq)
{
  frame_header_t h;
  memcpy(h.magic, "FRM2", 4);
  h.width = FRAME_W; h.height = FRAME_H; h.format = 1; h.seq = seq; h.length = FRAME_BYTES;
  h.crc32 = crc32_buf(0, frame, FRAME_BYTES);
  if (CDC_SendBlocking((const uint8_t *)&h, sizeof h, 500) != 0) return -1;
  return CDC_SendBlocking(frame, FRAME_BYTES, 2000);
}
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();
	
  /* Configure the MPU attributes for the QSPI 256MB without instruction access */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = OCTOSPI1_BASE;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure the MPU attributes for the QSPI 8MB (QSPI Flash Size) to Cacheable WT */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = OCTOSPI1_BASE;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  /* data only (model weights): no speculative instruction fetches while the flash is being written */
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
  /* Setup AXI SRAM in Cacheable WB */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = D1_AXISRAM_BASE;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  CPU_CACHE_Enable();
  MPU_Config();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  MX_USB_DEVICE_Init();
  board_button_init();
  board_led_init();
  LCD_Test();
  hwtest_init();                       /* servo closed, LEDs, buzzer, UART to the ESP32 */

  uint8_t text[40];
  int xf = xf_init();                  /* external flash: recognition weights + enrolled people */
  sprintf((char *)text, xf ? "Ext flash FAIL" : (xf_rec_valid() ? "Face model OK" : "No face model"));
  LCD_ShowString(4, 42, ST7735Ctx.Width, 16, 12, text);
  sprintf((char *)text, "Camera Not Found");
  LCD_ShowString(0, 58, ST7735Ctx.Width, 16, 16, text);
  Camera_Init_Device(&hi2c1, FRAMESIZE_QVGA);
  ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 58, ST7735Ctx.Width, 16, BLACK);
  sprintf((char *)text, "Cam id:0x%x", hcamera.device_id);
  LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
  ae_target = cam_rd(OV7725_AEW);
  if (ae_target < 0x20 || ae_target > 0xE0) set_ae_target(0x78);
  door_init();
  HAL_Delay(1000);

  uint16_t seq = 0;
  uint32_t last_fps_tick = HAL_GetTick(), frames = 0;
  start_capture();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    hwtest_poll();
    door_poll();

    if (DCMI_FrameIsReady && !door_busy())
    {
      /* The DMA wrote straight to RAM behind the D-cache: drop stale lines. */
      SCB_InvalidateDCache_by_Addr((uint32_t *)frame, FRAME_BYTES);
      auto_brightness_step();
      if (door_want_frame() && CDC_HostConnected()) send_frame(seq++);
      show_preview();
      door_process(frame);             /* face pipeline; the frame is gone afterwards */
      draw_overlay();

      frames++;
      if (HAL_GetTick() - last_fps_tick >= 1000)
      {
        Camera_FPS = frames; frames = 0; last_fps_tick = HAL_GetTick();
      }
      sprintf((char *)text, "%luFPS", (unsigned long)Camera_FPS);
      LCD_ShowString(2, 2, 40, 12, 12, text);
      board_led_toggle();
      start_capture();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 46;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_4);
}

/* USER CODE BEGIN 4 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  (void)hdcmi;
  DCMI_FrameIsReady = 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
