#include "serial.h"
eUSCI_UART_Config highFrequencyConfig =
{
	EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
	19, // BRDIV = 78
	8, // UCxBRF = 2
	0, // UCxBRS = 0
	EUSCI_A_UART_NO_PARITY, // No Parity
	EUSCI_A_UART_LSB_FIRST, // LSB First
	EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
	EUSCI_A_UART_MODE, // UART mode
	EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
};
eUSCI_UART_Config lowFrequencyConfig = {
	EUSCI_A_UART_CLOCKSOURCE_SMCLK,
	3, // BRDIV = 78
	0, // UCxBRF = 2
	3, // UCxBRS = 0
	EUSCI_A_UART_NO_PARITY, // No Parity
	EUSCI_A_UART_LSB_FIRST, // LSB First
	EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
	EUSCI_A_UART_MODE, // UART mode
	EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION
};
void initSerial(void)
{
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
	            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	if(MAP_UART_initModule(EUSCI_A0_BASE,&highFrequencyConfig))
	{
		MAP_UART_enableModule(EUSCI_A0_BASE);
		MAP_UART_enableInterrupt(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT|EUSCI_A_UART_BREAKCHAR_INTERRUPT);
		MAP_UART_clearInterruptFlag(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT|EUSCI_A_UART_BREAKCHAR_INTERRUPT);
		MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
	}
}
void writeDirect(uint8_t *data, int length)
{
	int a = 0;
	for(;a<length;a++)
	{
		MAP_UART_transmitData(EUSCI_A0_BASE,data[a]);
	}
}
uint8_t readByteDirect(){
	return MAP_UART_receiveData(EUSCI_A0_BASE);
}
