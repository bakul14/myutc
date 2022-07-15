/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static FLASH_EraseInitTypeDef Erase_struct;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_START_ADDRESS  0x08002400  //ADDR of main programm memory to jump

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN PV */
uint8_t FLAG_DOWNLOAD_START = 0;
uint8_t FLAG_DOWNLOAD_OVER = 0;
uint8_t FLAG_DEVICE_READY = 0;
uint8_t cell_ok = 0;
uint8_t buf_tx[8] = {'S','T','A','R','T','F','F','F'};
uint8_t buf_rx_0[8];
uint8_t buf_rx_1[8];
uint8_t buf_rx_2[8];
uint8_t word_1[8] = {0};
uint8_t word_2[8] = {0};
uint8_t word_3[8] = {0};
uint8_t word_4[8] = {0};
uint8_t response_msg = 1;
uint8_t cells_count = 0;
uint8_t status_can_tx = 0x01U;
uint8_t RXmsgData[8];
uint32_t erase_error = 0;
uint32_t program_data_1 = 0;
uint32_t program_data_2 = 0;
uint32_t program_data_3 = 0;
uint32_t program_data_4 = 0;
uint32_t data_tx[2];
uint32_t mailBox_0 = CAN_TX_MAILBOX0;
uint32_t mailBox_1 = CAN_TX_MAILBOX1;
uint32_t mailBox_2 = CAN_TX_MAILBOX2;
uint32_t counter_feedback_msg = 0;
uint32_t RAM_buf_counter = 0;
uint32_t RAM_buf_counter_2 = 0;
uint32_t RAM_buf[8000];
uint64_t count_pages = 0;
uint64_t address_data = APP_START_ADDRESS;
uint64_t address_data_2 = APP_START_ADDRESS;
uint8_t circuit_id = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Ascii_To_Hex( uint8_t* buff, uint8_t count) //??????? ???????? ???? ? ????
{
    uint8_t i;

    for(i=0; i<count; i++)
    {
        if(buff[i] <= '9' && buff[i] >= '0' )
        {
            buff[i] -= 0x30;
        }
        else
        {
            buff[i] = buff[i] - 0x41 + 10;
        }
    }
}

void reset_variables() {
    FLAG_DOWNLOAD_START = 0;
    FLAG_DOWNLOAD_OVER = 0;
    FLAG_DEVICE_READY = 0;
    cell_ok = 1;
    response_msg = 1;
    cells_count = 0;
    status_can_tx = 0x01U;

    erase_error = 0;
    program_data_1 = 0;
    program_data_2 = 0;
    program_data_3 = 0;
    program_data_4 = 0;
    counter_feedback_msg = 0;
    RAM_buf_counter = 0;
    RAM_buf_counter_2 = 0;
    count_pages = 0;

    circuit_id = 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */


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
    HAL_Delay(500);
    MX_USB_DEVICE_Init();
    MX_CAN1_Init();
    /* USER CODE BEGIN 2 */
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
 
    TxHeader.StdId = 0x200;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        if((FLAG_DOWNLOAD_OVER == 1) && (FLAG_DOWNLOAD_START == 0) && (FLAG_DEVICE_READY == 1)) { //???? ??????????? ???????? ???????? ? ?????????? ? ??????????? ?????????? ?????? ? ?????? ????????, ??

            while((RAM_buf_counter_2 <  RAM_buf_counter) && (cell_ok == 1)) { //???? ??????? ???????????? ? ??? ????????? ??? ?????? ?????????? ???? ???????? ? ??????? ????? ?? ???????????? ??????????, ??
                buf_tx[0] = RAM_buf[RAM_buf_counter_2] >> 24; // ?????? ? ????? ???????? ???? ?????? ?? ?????? ????????
                buf_tx[1] = RAM_buf[RAM_buf_counter_2] >> 16;
                buf_tx[2] = RAM_buf[RAM_buf_counter_2] >> 8;
                buf_tx[3] = RAM_buf[RAM_buf_counter_2];
                buf_tx[4] = RAM_buf[RAM_buf_counter_2+1] >> 24;
                buf_tx[5] = RAM_buf[RAM_buf_counter_2+1] >> 16;
                buf_tx[6] = RAM_buf[RAM_buf_counter_2+1] >> 8;
                buf_tx[7] = RAM_buf[RAM_buf_counter_2+1];
                RAM_buf_counter_2 += 2;
                status_can_tx = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buf_tx, &mailBox_0); // ???????? ????????? ? ???				// need to be changed!!!!!!!!!!!!!!!!!!!!
                cells_count += 2;
                cell_ok = 0; //????? ????? ???????? ????? ? ???????????
                while(status_can_tx != HAL_OK) { //???????? ?????????? ???????? ????????? ? ???

                }
            }
            if (RAM_buf_counter_2 >= RAM_buf_counter)	{ //???? ??????? ???????????? ????????? ????? ??? ????????? ??????? ???????? ? ?? ???? ???????, ??
                buf_tx[0] = 'E';
                buf_tx[1] = 'N';
                buf_tx[2] = 'D';
                HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buf_tx, &mailBox_0); //?????????? end ? ???????? ?? ??????????
                HAL_Delay(100); // ???? 100 ???
                reset_variables();
								for (int i = 0; i < 8000; i++) 
									RAM_buf[i] = 0;
                  NVIC_SystemReset(); // ????????????????
            }
        }
        else if(FLAG_DOWNLOAD_START == 1) { //???? ???????? ????????, ??
            TxHeader.StdId = 0x7F0 + circuit_id;
            HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buf_tx, &mailBox_0); // ?????????? ?????????? ? ???, ??? ???????? ????????
            FLAG_DOWNLOAD_START = 0; // ????? ????? ?????? ????????
        }
    }
}
/* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure the Systick interrupt time
    */
    __HAL_RCC_PLLI2S_ENABLE();
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 2;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */
    CAN_FilterTypeDef canFilterConfig;
    canFilterConfig.FilterBank = 0;
    canFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
    canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilterConfig.FilterIdHigh = 0x07F0<<5;
    canFilterConfig.FilterIdLow = 0x0000;
    canFilterConfig.FilterMaskIdHigh = 0x0000;
    canFilterConfig.FilterMaskIdLow = 0x0000;
    canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilterConfig.FilterActivation = ENABLE;
    canFilterConfig.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);
    /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    cell_ok = 1; //??? ??????? ????????? ???????? ????? ? ????????????????????? ???? ? 1
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RXmsgData); //
    if (counter_feedback_msg == 0) { //???? ?????? ???????? ?????????
        FLAG_DEVICE_READY = 1; //????????, ??? ??????????? ?????????? ?????? ? ????????????
    }
    cell_ok = 1;
    counter_feedback_msg++; // ??????????? ???? ???????? ?????????
    //CDC_Transmit_FS((uint8_t *)RXmsgData, strlen(RXmsgData));
    /*if(RXmsgData[0] == 'E' && RXmsgData[1] == 'N' && RXmsgData[2] == 'D')	{
    		HAL_FLASH_Lock();
    		HAL_GPIO_TogglePin(GPIOK, GPIO_PIN_3);
    		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_5);
    }
    else	{
    	strcpy(buf_rx, RXmsgData);
    	Ascii_To_Hex(buf_rx, 8);
    	for(int i=0; i<8;i=i+2)	{
    		buf_rx[i] <<= 4;
    		buf_rx[i] = buf_rx[i] | buf_rx[i+1];
    		program_data_1 |= buf_rx[i] <<(i*4);
    	}
    		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,  address_data, program_data_1);
    		for(int i2 = 0; i2 <= 7; i2++) {
    					buf_rx[i2] = 'F';
    					RXmsgData[i2] = 'F';
    				}
    		address_data = address_data + 4;
    		program_data_1 = 0;
    }*/
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
