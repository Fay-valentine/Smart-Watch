/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "user_TasksInit.h"
#include "user_LVGLTask.h"
#include "W25Q64.h"
#include "SerialTask.h"
#include "lvgl.h"
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
osMessageQueueId_t W25Q64QueueHandle;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for SerialTask */
osThreadId_t SerialTaskHandle;
const osThreadAttr_t SerialTask_attributes = {
  .name = "SerialTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for PageBackTask */
osThreadId_t PageBackTaskHandle;
const osThreadAttr_t PageBackTask_attributes = {
  .name = "PageBackTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};
/* Definitions for MsgInbox_Task */
osThreadId_t MsgInbox_TaskHandle;
const osThreadAttr_t MsgInbox_Task_attributes = {
  .name = "MsgInbox_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow7,
};
/* Definitions for IdleMonitorTask */
osThreadId_t IdleMonitorTaskHandle;
const osThreadAttr_t IdleMonitorTask_attributes = {
  .name = "IdleMonitorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow7,
};
/* Definitions for Sleep_Task */
osThreadId_t Sleep_TaskHandle;
const osThreadAttr_t Sleep_Task_attributes = {
  .name = "Sleep_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for WWDOG_Task */
osThreadId_t WWDOG_TaskHandle;
const osThreadAttr_t WWDOG_Task_attributes = {
  .name = "WWDOG_Task",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow7,
};
/* Definitions for W25Q64Task */
osThreadId_t W25Q64TaskHandle;
const osThreadAttr_t W25Q64Task_attributes = {
  .name = "W25Q64Task",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow6,
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* Definitions for IdleTimer */
osTimerId_t IdleTimerHandle;
const osTimerAttr_t IdleTimer_attributes = {
  .name = "IdleTimer"
};
/* Definitions for SerialMutex */
osMutexId_t SerialMutexHandle;
const osMutexAttr_t SerialMutex_attributes = {
  .name = "SerialMutex"
};
/* Definitions for W25Q64_RecursiveMutex */
osMutexId_t W25Q64_RecursiveMutexHandle;
const osMutexAttr_t W25Q64_RecursiveMutex_attributes = {
  .name = "W25Q64_RecursiveMutex",
  .attr_bits = osMutexRecursive,
};
/* Definitions for SerialBinarySem */
osSemaphoreId_t SerialBinarySemHandle;
const osSemaphoreAttr_t SerialBinarySem_attributes = {
  .name = "SerialBinarySem"
};
/* Definitions for SPI2_tx_BinarySem */
osSemaphoreId_t SPI2_tx_BinarySemHandle;
const osSemaphoreAttr_t SPI2_tx_BinarySem_attributes = {
  .name = "SPI2_tx_BinarySem"
};
/* Definitions for SPI2_rx_BinarySem */
osSemaphoreId_t SPI2_rx_BinarySemHandle;
const osSemaphoreAttr_t SPI2_rx_BinarySem_attributes = {
  .name = "SPI2_rx_BinarySem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartKeyTask(void *argument);
void StartSerialTask(void *argument);
void StartPageBackTask(void *argument);
void StartMsgInboxTask(void *argument);
void StartIdleMonitorTask(void *argument);
void StartSleepTask(void *argument);
void StartWWDOGTask(void *argument);
void StartSensorTask(void *argument);
void StartW25Q64Task(void *argument);
void IdleTimerCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationTickHook(void);

/* USER CODE BEGIN 3 */
void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
  TaskTickHook();
}
/* USER CODE END 3 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  User_Tasks_Init();
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of SerialMutex */
  SerialMutexHandle = osMutexNew(&SerialMutex_attributes);

  /* Create the recursive mutex(es) */
  /* creation of W25Q64_RecursiveMutex */
  W25Q64_RecursiveMutexHandle = osMutexNew(&W25Q64_RecursiveMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of SerialBinarySem */
  SerialBinarySemHandle = osSemaphoreNew(1, 0, &SerialBinarySem_attributes);

  /* creation of SPI2_tx_BinarySem */
  SPI2_tx_BinarySemHandle = osSemaphoreNew(1, 1, &SPI2_tx_BinarySem_attributes);

  /* creation of SPI2_rx_BinarySem */
  SPI2_rx_BinarySemHandle = osSemaphoreNew(1, 1, &SPI2_rx_BinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of IdleTimer */
  IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, &IdleTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  osTimerStart(IdleTimerHandle, 100);//100ms
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (16, sizeof(uint16_t), &myQueue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  const osMessageQueueAttr_t W25Q64Queue_attributes = {
    .name = "W25Q64Queue"
  };
  W25Q64QueueHandle = osMessageQueueNew(16,sizeof(StorageReq_t),&W25Q64Queue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* creation of SerialTask */
  SerialTaskHandle = osThreadNew(StartSerialTask, NULL, &SerialTask_attributes);

  /* creation of PageBackTask */
  PageBackTaskHandle = osThreadNew(StartPageBackTask, NULL, &PageBackTask_attributes);

  /* creation of MsgInbox_Task */
  MsgInbox_TaskHandle = osThreadNew(StartMsgInboxTask, NULL, &MsgInbox_Task_attributes);

  /* creation of IdleMonitorTask */
  IdleMonitorTaskHandle = osThreadNew(StartIdleMonitorTask, NULL, &IdleMonitorTask_attributes);

  /* creation of Sleep_Task */
  Sleep_TaskHandle = osThreadNew(StartSleepTask, NULL, &Sleep_Task_attributes);

  /* creation of WWDOG_Task */
  WWDOG_TaskHandle = osThreadNew(StartWWDOGTask, NULL, &WWDOG_Task_attributes);

  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of W25Q64Task */
  W25Q64TaskHandle = osThreadNew(StartW25Q64Task, NULL, &W25Q64Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  lv_mem_monitor_t lv_mon;
  /* Infinite loop */
  for(;;)
  {
    //监控内存使用情况
    // lv_mem_monitor(&lv_mon);
    // Serial_Printf("[MEM] LVGL used:%u total:%u max:%u frag:%u%% | FreeRTOS free:%u min:%u\r\n",
    //               (unsigned)(lv_mon.total_size - lv_mon.free_size),
    //               (unsigned)lv_mon.total_size,
    //               (unsigned)lv_mon.max_used,
    //               (unsigned)lv_mon.frag_pct,
    //               (unsigned)xPortGetFreeHeapSize(),
    //               (unsigned)xPortGetMinimumEverFreeHeapSize());
    //
    // osDelay(5000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
__weak void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartKeyTask */
}

/* USER CODE BEGIN Header_StartSerialTask */
/**
* @brief Function implementing the SerialTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSerialTask */
__weak void StartSerialTask(void *argument)
{
  /* USER CODE BEGIN StartSerialTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartSerialTask */
}

/* USER CODE BEGIN Header_StartPageBackTask */
/**
* @brief Function implementing the PageBackTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPageBackTask */
__weak void StartPageBackTask(void *argument)
{
  /* USER CODE BEGIN StartPageBackTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartPageBackTask */
}

/* USER CODE BEGIN Header_StartMsgInboxTask */
/**
* @brief Function implementing the MsgInbox_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMsgInboxTask */
__weak void StartMsgInboxTask(void *argument)
{
  /* USER CODE BEGIN StartMsgInboxTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartMsgInboxTask */
}

/* USER CODE BEGIN Header_StartIdleMonitorTask */
/**
* @brief Function implementing the IdleMonitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIdleMonitorTask */
__weak void StartIdleMonitorTask(void *argument)
{
  /* USER CODE BEGIN StartIdleMonitorTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartIdleMonitorTask */
}

/* USER CODE BEGIN Header_StartSleepTask */
/**
* @brief Function implementing the Sleep_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSleepTask */
__weak void StartSleepTask(void *argument)
{
  /* USER CODE BEGIN StartSleepTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartSleepTask */
}

/* USER CODE BEGIN Header_StartWWDOGTask */
/**
* @brief Function implementing the WWDOG_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWWDOGTask */
__weak void StartWWDOGTask(void *argument)
{
  /* USER CODE BEGIN StartWWDOGTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartWWDOGTask */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
__weak void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartW25Q64Task */
/**
* @brief Function implementing the W25Q64Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartW25Q64Task */
__weak void StartW25Q64Task(void *argument)
{
  /* USER CODE BEGIN StartW25Q64Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartW25Q64Task */
}

/* IdleTimerCallback function */
__weak void IdleTimerCallback(void *argument)
{
  /* USER CODE BEGIN IdleTimerCallback */

  /* USER CODE END IdleTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

