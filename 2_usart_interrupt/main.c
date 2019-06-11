#include <stdint.h>
#include "reg.h"
#include "blink.h"

void init_usart1(void)
{
	// PB6: USART1_Tx 
	// PB7: USART1_Rx

	//RCC EN GPIOB
	SET_BIT(RCC_BASE + RCC_AHB1ENR_OFFSET, GPIO_EN_BIT(GPIO_PORTB));

	//GPIO Configurations
	//MODER  => General purpose output mode
	//MODER 10: Alternate function mode
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_1_BIT(6)); //PB6
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_0_BIT(6));

	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_1_BIT(7)); //PB7
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_MODER_OFFSET, MODERy_0_BIT(7));

	//OT led pin = 0 => Output push-pull
	// OTYPER 0
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(6)); //PB6
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OTYPER_OFFSET, OTy_BIT(7)); //PB7

	//OSPEEDR led pin = 00 => Low speed
	// OSPEEDR led pin = 01 => Medium speed
	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(6)); //PB6
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(6));

	SET_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_1_BIT(7)); //PB7
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_OSPEEDR_OFFSET, OSPEEDRy_0_BIT(7));

	//PUPDR led pin = 00 => No pull-up, pull-down
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_1_BIT(6)); //PB6
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_0_BIT(6));

	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_1_BIT(7)); //PB7
	CLEAR_BIT(GPIO_BASE(GPIO_PORTB) + GPIOx_PUPDR_OFFSET, PUPDRy_0_BIT(7));

        //AF mode = AF7
	WRITE_BITS(GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(6), AFRLy_0_BIT(6), 7); //PB6
	WRITE_BITS(GPIO_BASE(GPIO_PORTB) + GPIOx_AFRL_OFFSET, AFRLy_3_BIT(7), AFRLy_0_BIT(7), 7); //PB7

	//RCC EN USART1
	SET_BIT(RCC_BASE + RCC_APB2ENR_OFFSET, USART1EN);// APB enable USAR



	//Baud
	const unsigned int BAUD = 115200;
	const unsigned int SYSCLK_MHZ = 168;
	const double USARTDIV = SYSCLK_MHZ * 1.0e6 / 16 / BAUD;

	const uint32_t DIV_MANTISSA = (uint32_t)USARTDIV;
	const uint32_t DIV_FRACTION = (uint32_t)((USARTDIV-DIV_MANTISSA) * 16);

	//USART  Configurations
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, UE_BIT);// enable USART
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET , DIV_MANTISSA_11_BIT , DIV_MANTISSA_0_BIT , DIV_MANTISSA);// set mantissa for baudrate
	WRITE_BITS(USART1_BASE + USART_BRR_OFFSET , DIV_FRACTION_3_BIT , DIV_FRACTION_0_BIT , DIV_FRACTION);// set fraction for baudrate
     
	//UART EN
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, TE_BIT);// enable transimitter
	SET_BIT(USART1_BASE + USART_CR1_OFFSET, RE_BIT);// enable receiver

	SET_BIT(USART1_BASE + USART_CR1_OFFSET, RXNEIE_BIT); // Set RXNEIE bit in USART_CR1 register to enable RXNE interrupt
	// NVIC configuration (m+(32*n)) | m=5,n=1
	SET_BIT(NVIC_ISER_BASE + NVIC_ISERn_OFFSET(1), 5); // Enable IRQ37
}

void usart1_send_char(const char ch)
{
	//wait until TXE==1(TXE bit is set)
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET,TXE_BIT))
	;
	// Write the data to send in the USART_DR register (this clears the TXE bit)
	REG(USART1_BASE + USART_DR_OFFSET)=ch;
    //	blink_count(LED_GREEN,1);

}

char usart1_receive_char(void)
{
	// When a character is received, the RXNE bit is set
	while(!READ_BIT(USART1_BASE + USART_SR_OFFSET, RXNE_BIT))
	;
	// Read to the USART_DR register (this clears the RXNE bit)
	return (char) REG(USART1_BASE + USART_DR_OFFSET);

}

void usart1_handler() {
	// send a '~' if overrun happens
	if(READ_BIT(USART1_BASE + USART_SR_OFFSET, ORE_BIT)) {
		usart1_send_char('~');
		blink_count(LED_RED, 10);
	}
	// When a character is received, the RXNE bit is set
	else if(READ_BIT(USART1_BASE + USART_SR_OFFSET, RXNE_BIT)) {
		// Read to the USART_DR register (this clears the RXNE bit)
		char ch = usart1_receive_char();

		if (ch == '\r')
			usart1_send_char('\n');

		usart1_send_char(ch);
	}
}
/*
void usart1_handler(void)
{
        char ch;
		
	if(!(READ_BIT(USART1_BASE + USART_SR_OFFSET , ORE_BIT)))
	{		
		ch = usart1_receive_char();

		if (ch == '\r')
			usart1_send_char('\n');

		usart1_send_char(ch);
	}
	else
	{
        char *OREERROR = "ORE ERROR\r\n";

	//send ORE ERROR
	while (*OREERROR != '\0')
		usart1_send_char(*OREERROR++);
        ch = usart1_receive_char();
	}
	while((READ_BIT(USART1_BASE + USART_SR_OFFSET , ORE_BIT)))
        {}


}
*/

int main(void)
{
	init_usart1();

	char *hello = "Hello world!\r\n";

	//send Hello world
	while (*hello != '\0')
		usart1_send_char(*hello++);


	//receive char and resend it
	char ch;
	while (1)
	{
		ch = usart1_receive_char();

		if (ch == '\r')
			usart1_send_char('\n');

		usart1_send_char(ch);
	}
}
