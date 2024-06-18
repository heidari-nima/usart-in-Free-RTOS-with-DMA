# usart-with-Free-RTOS-with-DMA
usart with Free RTOS with DMA
[report.pdf](https://github.com/user-attachments/files/15879025/report.pdf)
Introduction to UART Communication and Its Applications

UART, or Universal Asynchronous Receiver-Transmitter, is a widely used hardware communication protocol that facilitates serial communication between devices. It is a crucial component in embedded systems and microcontroller communications. Unlike synchronous protocols, UART does not require a shared clock signal, making it simpler and more cost-effective to implement.

Key Features of UART:
1. Asynchronous Communication: Data is transmitted without a shared clock signal.
2. Serial Data Transfer: Data bits are sent sequentially over a single channel.
3. Start and Stop Bits: These bits frame the data packet, ensuring correct data interpretation.
4. Baud Rate: The speed of communication is defined by the baud rate, which must be agreed upon by both communicating devices.
5. Error Detection: Parity bits can be used for simple error detection.

 Applications of UART:
1. Embedded Systems: UART is commonly used for communication between microcontrollers and peripherals such as sensors, displays, and memory devices.
2. Computers and Peripherals: It connects PCs to devices like modems, GPS modules, and serial printers.
3. Communication Modules: Used in Bluetooth, Zigbee, and GSM modules for wireless communication.
4. Debugging and Development: Engineers use UART for serial communication during the development and debugging of embedded systems, providing a simple interface for logging data and interacting with the system.
5. Industrial Automation: Facilitates communication between controllers, sensors, and actuators in industrial settings.

UART's simplicity, ease of implementation, and reliable data transmission make it a versatile choice for a wide range of communication applications in various fields.

![image](https://github.com/heidari-nima/usart-with-Free-RTOS-with-DMA/assets/79926326/2c029c6c-9c6e-4eca-a441-fe60d1f312fa)

Reception using DMA DMA mode can be enabled for reception by setting the DMAR bit in USART_CR3 register. Data is loaded from the USART_DR register to a SRAM area configured using the DMA peripheral (refer to the DMA specification) whenever a data byte is received. To map a DMA channel for USART reception, use the following procedure:
 1. Write the USART_DR register address in the DMA control register to configure it as the source of the transfer. The data will be moved from this address to the memory after each RXNE event. 
2. Write the memory address in the DMA control register to configure it as the destination of the transfer. The data will be loaded from USART_DR to this memory area after each RXNE event. 
3. Configure the total number of bytes to be transferred in the DMA control register. 
4. Configure the channel priority in the DMA control register 
5. Configure interrupt generation after half/ full transfer as required by the application. 
6. Activate the channel in the DMA control register. When the number of data transfers programmed in the DMA Controller is reached, the DMA controller generates an interrupt on the DMA channel interrupt vector.
![image](https://github.com/heidari-nima/usart-with-Free-RTOS-with-DMA/assets/79926326/6af996cc-dbe9-4ce0-81c0-1a4259a73e45)
![image](https://github.com/heidari-nima/usart-with-Free-RTOS-with-DMA/assets/79926326/c39bfe90-4fe5-44d2-bf21-a3cd11d23de1)


In this project we use idle line detect interrupt for understanding  when data receiving is ended  
