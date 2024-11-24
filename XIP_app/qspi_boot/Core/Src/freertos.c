/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <delay.h>
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
/* USER CODE BEGIN Variables */
typedef  void (*iapfun)(void);  

uint8_t jumpFlag = 0;

#define FLASH_APP_ADDR		0x90000000  

osThreadId jumpTaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartJumpTask(void const * argument);

iapfun jump2app;

//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(uint32_t addr){
    asm("MSR MSP, r0");			//set Main Stack value
    asm("BX r14");
}

void iap_load_app(uint32_t appxaddr){
	if(((*(uint32_t*)appxaddr)&0x2FF00000)==0x20000000)	//检查主堆栈指针指向位置是否合法
	{ 
		// SysTick->CTRL = 0X00;//禁止SysTick
		// SysTick->LOAD = 0;
		// SysTick->VAL = 0;

		// HAL_SuspendTick();

		//__disable_irq();

		portDISABLE_INTERRUPTS();	//如果开启rtos

		jump2app=(iapfun)*(__IO uint32_t*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)

		MSR_MSP(*(uint32_t*)appxaddr);					//设置APP的主堆栈指针

		//__set_MSP(*(__IO uint32_t*) appxaddr);
		jump2app();									//��ת��APP.
	}
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 2048);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(jumpTask, StartJumpTask, osPriorityNormal, 0, 2048);
  jumpTaskHandle = osThreadCreate(osThread(jumpTask), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  printf("bootloader defult task started!!!\r\n");

  for(;;)
  {
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_SET);
	  osDelay(500);
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_RESET);
	  osDelay(500);

	  printf("hello bootloader\r\n");

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartJumpTask(void const * argument)
{
  //printf("StartJumpTask started!!!\r\n");
  while(1){
    if(jumpFlag){
      printf("ready to jump!!!\r\n");
      osDelay(50);
      jumpFlag = 0;

      if (CSP_QSPI_EnableMemoryMappedMode() != HAL_OK) {
        while (1)
          ; //breakpoint - error detected
      }

      delay_ms(20);

      // uint8_t *temp = (uint8_t*)0x90000000;
      // for(int i=0; i<4; ++i){
      //   printf("%02X\r\n", temp[i]);  //两位宽度，0补齐
      // }

      iap_load_app(FLASH_APP_ADDR);
      while (1)
      {
        ;
      }
      
    }
//    else{
//    	printf("nothing\r\n");
//    }

    osDelay(500);
  }
}
/* USER CODE END Application */

