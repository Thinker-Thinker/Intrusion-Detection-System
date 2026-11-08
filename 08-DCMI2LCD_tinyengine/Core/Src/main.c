/* USER CODE BEGIN Header */

/*---------------------------------------
- WeAct Studio Official Link
- taobao: weactstudio.taobao.com
- aliexpress: weactstudio.aliexpress.com
- github: github.com/WeActTC
- gitee: gitee.com/WeAct-TC
- blog: www.weact-tc.cn
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
#include <stdio.h>
#include <string.h>
#include "genNN.h"
#include "tinyengine_function.h"
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
#ifdef TFT96
// QQVGA
#define FrameWidth 160
#define FrameHeight 120
#elif TFT18
// QQVGA2
#define FrameWidth 128
#define FrameHeight 160
#endif
// picture buffer
// Written by DMA into cacheable AXI SRAM: align to the 32-byte cache line so it
// can be invalidated without touching neighbouring variables.
__attribute__((aligned(32))) uint16_t pic[FrameWidth][FrameHeight];
volatile uint32_t DCMI_FrameIsReady = 0;
uint32_t Camera_FPS=0;

// TinyEngine model input: 80x80 RGB, int8 (pixel - 128), HWC order
#define RES_W 80
#define RES_H 80
float labels[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
// Screen-sized copy of the camera frame that the result text is drawn onto.
// Kept in the camera's byte order (high byte first) so it goes straight to the LCD.
#ifdef TFT96
#define LCD_W 160
#define LCD_H 80
#define PREVIEW_OFFSET (20 * FrameHeight) // same crop the original preview used
#elif TFT18
#define LCD_W 128
#define LCD_H 160
#define PREVIEW_OFFSET 0
#endif
static uint16_t frame_view[LCD_W * LCD_H];

// 6x12 ASCII font defined in Bsp/ST7735/font.h (compiled into lcd.c)
extern const unsigned char asc2_1206[95][12];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 * Camera frame (RGB565, high byte first as DCMI stores it) -> TinyEngine input.
 * Center-crops the largest square from the frame, resizes it to 80x80 with
 * nearest-neighbour sampling, and writes int8 RGB (value - 128) in HWC order,
 * the same format the TinyEngine VWW tutorial feeds the model.
 */
static void camera_to_tinyengine(const uint16_t *frame, signed char *input)
{
  const int crop = (FrameWidth < FrameHeight) ? FrameWidth : FrameHeight;
  const int x0 = (FrameWidth - crop) / 2;
  const int y0 = (FrameHeight - crop) / 2;

  for (int y = 0; y < RES_H; y++)
  {
    const uint16_t *row = frame + (y0 + y * crop / RES_H) * FrameWidth;
    for (int x = 0; x < RES_W; x++)
    {
      uint16_t raw = row[x0 + x * crop / RES_W];
      uint16_t p = (uint16_t)((raw >> 8) | (raw << 8)); // to native RGB565

      uint8_t r5 = (p >> 11) & 0x1F;
      uint8_t g6 = (p >> 5) & 0x3F;
      uint8_t b5 = p & 0x1F;
      int r = (r5 << 3) | (r5 >> 2);
      int g = (g6 << 2) | (g6 >> 4);
      int b = (b5 << 3) | (b5 >> 2);

      signed char *dst = &input[(y * RES_W + x) * 3];
      dst[0] = (signed char)(r - 128);
      dst[1] = (signed char)(g - 128);
      dst[2] = (signed char)(b - 128);
    }
  }
}

/*
 * Draw a string into an LCD_W x LCD_H frame using the 6x12 font.
 * Font layout (same as LCD_ShowChar): 6 columns per glyph, each column is
 * 12 bits spread over two bytes, MSB = top pixel. Only set pixels are written,
 * so the image underneath stays visible.
 */
static void draw_text(uint16_t *fb, int x, int y, const char *s, uint16_t color)
{
  const uint16_t c = (uint16_t)((color >> 8) | (color << 8)); // LCD byte order
  for (; *s; s++, x += 6)
  {
    if (*s < ' ' || *s > '~')
      continue;
    const unsigned char *glyph = asc2_1206[*s - ' '];
    int col = 0, row = 0;
    for (int t = 0; t < 12; t++)
    {
      uint8_t bits = glyph[t];
      for (int b = 0; b < 8; b++, bits <<= 1)
      {
        int px = x + col, py = y + row;
        if ((bits & 0x80) && px >= 0 && px < LCD_W && py >= 0 && py < LCD_H)
          fb[py * LCD_W + px] = c;
        if (++row == 12)
        {
          row = 0;
          col++;
          break;
        }
      }
    }
  }
}

// Text with a 1-pixel black shadow so it stays readable on any background
static void overlay_text(uint16_t *fb, int x, int y, const char *s, uint16_t color)
{
  draw_text(fb, x + 1, y + 1, s, BLACK);
  draw_text(fb, x, y, s, color);
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
  char buf[150];
  char showbuf[150];
  /* USER CODE BEGIN 1 */


  CPU_CACHE_Enable();
//  MPU_Config();
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
	board_button_init();
	board_led_init();
	
	LCD_Test();
	
	uint8_t text[30];

  sprintf((char *)&text, "Camera Not Found");
  LCD_ShowString(0, 58, ST7735Ctx.Width, 16, 16, text);

  #ifdef TFT96
	Camera_Init_Device(&hi2c1, FRAMESIZE_QQVGA);
	#elif TFT18
	Camera_Init_Device(&hi2c1, FRAMESIZE_QQVGA2);
	#endif
	//clean Ypos 58
	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 58, ST7735Ctx.Width, 16, BLACK);
	
  // while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
  {

    sprintf((char *)&text, "Camera id:0x%x        ", hcamera.device_id);
    LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
		
		HAL_Delay(500);
		
    sprintf((char *)&text, "LongPress K1 to Run");
    LCD_ShowString(4, 58, ST7735Ctx.Width, 16, 12, text);
		
		HAL_Delay(500);
  }

  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)&pic, FrameWidth * FrameHeight * 2 / 4);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (DCMI_FrameIsReady)
    {
        DCMI_FrameIsReady = 0;
        // Drop stale cache lines so the CPU reads the frame the DMA just wrote
        SCB_InvalidateDCache_by_Addr((uint32_t *)pic, sizeof(pic));

        // Camera frame -> model input, then run inference
        uint32_t start = HAL_GetTick();
        camera_to_tinyengine(&pic[0][0], getInput());
        invoke(labels);
        signed char *output = getOutput();
        int person = (output[1] > output[0]); // class 0: no person, class 1: person
        uint32_t infer_ms = HAL_GetTick() - start;

        // Full-screen camera image with the result drawn on top
        memcpy(frame_view, (const uint16_t *)pic + PREVIEW_OFFSET, sizeof(frame_view));
        overlay_text(frame_view, 3, 2, person ? "PERSON" : "NO PERSON", person ? RED : GREEN);
        char line[32];
        snprintf(line, sizeof(line), "%lums  %luFPS", (unsigned long)infer_ms, (unsigned long)Camera_FPS);
        overlay_text(frame_view, 3, LCD_H - 14, line, WHITE);
        ST7735_FillRGBRect(&st7735_pObj, 0, 0, (uint8_t *)frame_view, LCD_W, LCD_H);

        board_led_toggle();
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
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

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
	static uint32_t count = 0,tick = 0;

	if(HAL_GetTick() - tick >= 1000)
	{
		tick = HAL_GetTick();
		Camera_FPS = count;
		count = 0;
	}
	count ++;
	
  DCMI_FrameIsReady ++;
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
#ifdef USE_FULL_ASSERT
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
