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
 * 153.6 KB, so it lives in the 320 KB AXI SRAM (DMA1 cannot reach DTCM). */
#define FRAME_W 320
#define FRAME_H 240
#define FRAME_BYTES (FRAME_W * FRAME_H * 2)

/* 0.96" ST7735 panel: 160x80 landscape. Preview = 2x downscale, centre crop. */
#define PREVIEW_W 160
#define PREVIEW_H 80

__attribute__((section(".axi_sram"), aligned(32))) static uint8_t frame[FRAME_BYTES];
static uint16_t preview[PREVIEW_W * PREVIEW_H];

volatile uint32_t DCMI_FrameIsReady = 0;
uint32_t Camera_FPS = 0;

/* Frame header sent before every frame on the USB virtual COM port.
 * Little-endian, 16 bytes. The host tool (tools/capture.py) parses it. */
typedef struct __attribute__((packed)) {
  char     magic[4];   /* "FRM1" */
  uint16_t width;
  uint16_t height;
  uint16_t format;     /* 1 = RGB565, bytes in sensor order (high byte first) */
  uint16_t seq;
  uint32_t length;     /* payload bytes that follow */
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
  const uint16_t *src = (const uint16_t *)frame;
  for (int y = 0; y < PREVIEW_H; y++)
  {
    const uint16_t *row = &src[(2 * (y + 20)) * FRAME_W];
    for (int x = 0; x < PREVIEW_W; x++)
      preview[y * PREVIEW_W + x] = row[2 * x];
  }
  ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)preview, PREVIEW_W, PREVIEW_H);
}

static void send_text(const char *msg)
{
  CDC_SendBlocking((const uint8_t *)msg, (uint32_t)strlen(msg), 200);
}

static int send_frame(uint16_t seq)
{
  frame_header_t h;
  memcpy(h.magic, "FRM1", 4);
  h.width = FRAME_W; h.height = FRAME_H; h.format = 1; h.seq = seq; h.length = FRAME_BYTES;
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
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
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

  uint8_t text[40];
  sprintf((char *)text, "Camera Not Found");
  LCD_ShowString(0, 58, ST7735Ctx.Width, 16, 16, text);
  Camera_Init_Device(&hi2c1, FRAMESIZE_QVGA);
  ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 58, ST7735Ctx.Width, 16, BLACK);
  sprintf((char *)text, "Cam id:0x%x", hcamera.device_id);
  LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
  HAL_Delay(1000);

  uint8_t streaming = 0;   /* 'C' = continuous, 'P' = pause, 'S' = one frame */
  int8_t  send_next = 0;
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
    int c;
    while ((c = CDC_ReadByte()) >= 0)
    {
      switch (c)
      {
      case 'S': case 's': send_next = 1; break;
      case 'C': case 'c': streaming = 1; break;
      case 'P': case 'p': streaming = 0; break;
      case 'I': case 'i':
      {
        char info[96];
        snprintf(info, sizeof info, "INFO cam=0x%04x w=%d h=%d fmt=RGB565 fps=%lu\n",
                 hcamera.device_id, FRAME_W, FRAME_H, (unsigned long)Camera_FPS);
        send_text(info);
        break;
      }
      default: break;
      }
    }

    if (DCMI_FrameIsReady)
    {
      /* The DMA wrote straight to RAM behind the D-cache: drop stale lines. */
      SCB_InvalidateDCache_by_Addr((uint32_t *)frame, FRAME_BYTES);

      if ((streaming || send_next) && CDC_HostConnected())
      {
        if (send_frame(seq++) != 0) streaming = 0;   /* host stopped reading */
        send_next = 0;
      }
      show_preview();

      frames++;
      if (HAL_GetTick() - last_fps_tick >= 1000)
      {
        Camera_FPS = frames; frames = 0; last_fps_tick = HAL_GetTick();
      }
      sprintf((char *)text, "%luFPS %s", (unsigned long)Camera_FPS, streaming ? "USB" : "");
      LCD_ShowString(5, 5, 80, 16, 12, text);
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
