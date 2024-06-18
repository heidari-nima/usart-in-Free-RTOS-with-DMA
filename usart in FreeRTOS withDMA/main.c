#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "string.h"
#include "stdio.h"


// Function Prototypes
void SystemClock_Config(void);
void GPIO_Init(void);
void UART1_Init(void);
void DMA_Init(void);
void UART_Task(void *pvParameters);
void DMA1_Channel4_IRQHandler(void);
//void DMA1_Channel5_IRQHandler(void);
void USART1_IRQHandler(void);

// Buffer for received data
char rx_buffer[100];
char tx_buffer[100];
volatile uint8_t rx_complete = 0;

// Semaphore for UART DMA TX complete
SemaphoreHandle_t txCompleteSemaphore;
// Semaphore to signal that new data has been received
SemaphoreHandle_t rxDataSemaphore;

int main(void)
{
    // Configure the system clock
    SystemClock_Config();
    
    // Initialize GPIO for UART
    GPIO_Init();
    
    // Initialize UART1
    UART1_Init();
    
    // Initialize DMA
    DMA_Init();
    
    // Create binary semaphore for TX complete
    txCompleteSemaphore = xSemaphoreCreateBinary();
    
	
		rxDataSemaphore = xSemaphoreCreateBinary();
    // Create UART task
    xTaskCreate(UART_Task, "UART_Task", 128, NULL, 1, NULL);
    
    // Start scheduler
    vTaskStartScheduler();
    
    // The code should never reach here
    while (1)
    {
    }
}

void SystemClock_Config(void)
{
    // Enable HSE (High-Speed External) clock
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    
    // Set PLL source and multiplier   external clock is 8 MHz   8*9=72Mhz
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9;
    
    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)); // wait for locking PLL
    
    // Set PLL as system clock source
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    // Update SystemCoreClock variable
    SystemCoreClockUpdate();
}

void GPIO_Init(void)
{
    // Enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
    // Configure PA9 (USART1_TX) as alternate function push-pull
    GPIOA->CRH &= ~GPIO_CRH_CNF9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;
    GPIOA->CRH |= GPIO_CRH_MODE9;
    
    // Configure PA10 (USART1_RX) as input floating
    GPIOA->CRH &= ~GPIO_CRH_CNF10;
    GPIOA->CRH |= GPIO_CRH_CNF10_0;
    GPIOA->CRH &= ~GPIO_CRH_MODE10;
}

void UART1_Init(void)
{
    // Enable USART1 clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    
		// set baud rate varilbe
		uint32_t baudRate =9600;
    // Configure baud rate (assuming 72 MHz system clock)
    USART1->BRR = SystemCoreClock / baudRate;
    
    // Enable USART1, transmitter, receiver, and DMA for both TX and RX
    USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE| USART_CR1_IDLEIE;
	  NVIC_EnableIRQ(USART1_IRQn);
}

void DMA_Init(void)
{
    // Enable DMA1 clock
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    
    // Configure DMA for USART1_TX (DMA1 Channel 4)
    DMA1_Channel4->CCR = 0;
    DMA1_Channel4->CCR |= DMA_CCR4_MINC | DMA_CCR4_DIR | DMA_CCR4_TCIE;//| DMA_CCR5_CIRC; // Memory increment mode, read from memory, transfer complete interrupt , circular mode
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR; // Set peripheral address (USART1 data register)
    NVIC_EnableIRQ(DMA1_Channel4_IRQn); // Enable DMA1 Channel4 interrupt in NVIC

    // Configure DMA for USART1_RX (DMA1 Channel 5)
    DMA1_Channel5->CCR = 0;
    DMA1_Channel5->CCR |= DMA_CCR5_MINC | DMA_CCR5_TCIE| DMA_CCR5_CIRC; // Memory increment mode, transfer complete interrupt, circular mode
    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR; // Set peripheral address (USART1 data register)
    DMA1_Channel5->CMAR = (uint32_t)rx_buffer; // Set memory address
    DMA1_Channel5->CNDTR = sizeof(rx_buffer); // Set number of data items
    DMA1_Channel5->CCR |= DMA_CCR5_EN; // Enable DMA channel
    NVIC_EnableIRQ(DMA1_Channel5_IRQn); // Enable DMA1 Channel5 interrupt in NVIC
}

void UART_Task(void *pvParameters)
{
     static uint16_t old_pos = 0;

    while (1) {
        // Wait until new data is received
        if (xSemaphoreTake(rxDataSemaphore, portMAX_DELAY) == pdTRUE) {
            uint16_t pos = sizeof(rx_buffer) - DMA1_Channel5->CNDTR;

            if (pos != old_pos) {
                if (pos > old_pos) {
                    memcpy(tx_buffer, &rx_buffer[old_pos], pos - old_pos);
                } else {
                    memcpy(tx_buffer, &rx_buffer[old_pos], sizeof(rx_buffer) - old_pos);
                    memcpy(&tx_buffer[sizeof(rx_buffer) - old_pos], rx_buffer, pos);
                }

                old_pos = pos;
								 DMA1_Channel4->CCR &= ~DMA_CCR4_EN;
                 DMA1_Channel4->CMAR = (uint32_t)tx_buffer;
                 DMA1_Channel4->CNDTR = strlen(tx_buffer);
                 DMA1_Channel4->CCR |= DMA_CCR4_EN;
                // Wait for the previous transmission to complete
                if (xSemaphoreTake(txCompleteSemaphore, portMAX_DELAY) == pdTRUE) {
									memset(tx_buffer,0,sizeof(tx_buffer));
                }
            }
					}
    }
}

// DMA1 Channel4 interrupt handler (for USART1_TX)
void DMA1_Channel4_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF4)
    {
        // Clear transfer complete flag
        DMA1->IFCR |= DMA_IFCR_CTCIF4;
        
        // Release the semaphore
        xSemaphoreGiveFromISR(txCompleteSemaphore, NULL);
    }
}



/*IDLE: IDLE line detected
This bit is set by hardware when an Idle Line is detected. An interrupt is generated if the 
IDLEIE=1 in the USART_CR1 register. It is cleared by a software sequence (an read to the 
USART_SR register followed by a read to the USART_DR register). 
0: No Idle Line is detected
1: Idle Line is detected
Note: The IDLE bit will not be set again until the RXNE bit has been set itself (i.e. a new idle 
line occurs).*/
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        (void) USART1->SR;
        (void) USART1->DR;
        rx_complete = 1;
        xSemaphoreGiveFromISR(rxDataSemaphore, NULL);
    }
}

void Error_Handler(void)
{
    // Handle error
    while(1)
    {
        // Stay in infinite loop in case of error
    }
}
