#include "S32K144.h"
#include "clocks_and_modes.h"

#define PTD0  	0     		//Init LED BLUE
#define PTD15 	15			//Init LED RED

#define PTD16  	16			//Init LED GREEN

#define PTC12 	12			//Init BUTTON BTN1		
#define PTC13	13			//Init BUTTON BTN2	

/* SysTick Init with busy wait running at FIRC CLOCK.*/
#define S32_SysTick_CSR		0xE000E010
#define S32_SysTick_RVR  	0xE000E014
#define S32_SysTick_CVR 	0xE000E018

/*[31:24]	PRI_15	Priority of system handler 15, SysTick exception */
#define	S32_SCB_SHPR3			0xE000ED20  	/*Syst ctrl block, Find in arm infocenter web manual*/
#define S32_SCB_SHPR3_CNFVAL 	0x40000000
#define S32_SCB_SHPR3_MASK 		0x00FFFFFF
#define STCR_RELVAL		 		(16000000-1)	/*Reload value*/
#define S32_CLR					0x00000000
#define	SysTick_CSR_MASK		0x00000007



#define LPIT_Clock_1ms		40000
int time = 0 ;
int TT_Buttton;
int stt = 0 ;
void Delay_ms(unsigned int delay);
void SysTick_Init(void)
{
	S32_SysTick->CSR = 0;
	S32_SysTick->RVR = STCR_RELVAL;
	S32_SysTick->CVR = S32_CLR;
	S32_SCB->SHPR3= (S32_SCB->SHPR3 & 0x00FFFFFF)|0x40000000;
	S32_SysTick->CSR = 0x07;		//CLKSOURCE =1 processor clock source,
									//TICKINT=1 counting down to zero asserts Systick exception,
						 	 	 	//ENABLE= 1 counter enabled.
}
void NVIC_init_IRQs (void)
{
	S32_NVIC->ICPR[1] = 1 << (48 % 32);  /* IRQ49-LPIT0 ch1: clr any pending IRQ*/

	S32_NVIC->ISER[1] = 1 << (48 % 32);  /* IRQ49-LPIT0 ch1: enable IRQ */

	S32_NVIC->IP[48] = 0xA;              /* IRQ49-LPIT0 ch1: priority 10 of 0-15*/


	S32_NVIC->ICPR[1] = 1 << (61 % 32);  /* IRQ61-PORT C : clr any pending IRQ*/

	S32_NVIC->ISER[1] = 1 << (61 % 32);  /* IRQ61-PORT C : enable IRQ */

	S32_NVIC->IP[61] = 0x9;              /* IRQ61-PORT C : priority 1 of 0-15*/


	S32_NVIC->ICPR[1] = 1 << (15 % 32);  /* IRQ15-SYSTICK : clr any pending IRQ*/

	S32_NVIC->ISER[1] = 1 << (15 % 32);  /* IRQ15-SYSTICK : enable IRQ */

	S32_NVIC->IP[15] = 0x1;
}
void LED_Init(void)
{
  PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
  PTD->PDDR |= 1<<PTD0
			  |1<<PTD15
			  |1<<PTD16;            		/* Port D0,15,16:  Data Direction is output */
  PORTD->PCR[0] |=  PORT_PCR_MUX(1);  	/* Port D0:  MUX = ALT1, GPIO (to blue LED on Board) */
  PORTD->PCR[15] |=  PORT_PCR_MUX(1);  	/* Port D15:  MUX = ALT1, GPIO (to blue LED on Board) */
  PORTD->PCR[16] |=  PORT_PCR_MUX(1);  	/* Port D16:  MUX = ALT1, GPIO (to blue LED on Board) */

}
void  __Blink_Led_OFF(unsigned int port)
{
	PTD-> PCOR |= 1<< port;			//Set Output on port D0 (LED ON)
}
void __Blink_Led_ON(unsigned int port)
{
	PTD-> PSOR |= 1<< port;			//Set Output on port D0 (LED ON)
}
void Button_Init(void)
{
	PCC-> PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;/* Enable clocks to peripherals (PORT modules) */

	PTC->PDDR &= ~(1<<PTC12);   /* Port C12: Data Direction= input (default) */
	PORTC->PCR[12] |= PORT_PCR_MUX(1)
					|PORT_PCR_PFE_MASK; /* Port C12: MUX = GPIO, input filter enabled */


	PTC->PDDR &= ~(1<<PTC13);   /* Port C13: Data Direction= input (default) */
	PORTC->PCR[13] |= PORT_PCR_MUX(1)
					|PORT_PCR_PFE_MASK		/* Port C13: MUX = GPIO, input filter enabled */
					|PORT_PCR_IRQC_MASK		/*Enable Port C interrupt*/
					|PORT_PCR_IRQC(8);
}

void BTN_Control_Led()
{
	if (PTC->PDIR & (1<<PTC12))			// BTN 1
	{
		TT_Buttton = 1 ; 
	}
}


void LPIT0_init (void)
{
  PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
  PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs 		*/
  /*!
   * LPIT Initialization:
   */
  LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
                              	  	  /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              	  	  /* SW_RST=0: SW reset does not reset timer chans, regs */
                              	  	  /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
  LPIT0->MIER = LPIT_MIER_TIE0_MASK;  /* TIE0=1: Timer Interrupt Enabled fot Chan 0 */

  LPIT0->TMR[0].TVAL = LPIT_Clock_1ms;      /* Chan 0 Timeout period: 40M clocks */

  //LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
  	  	  	  	  	  	  	  /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}


void WDOG_disable (void)
{
  WDOG->CNT		=0xD928C520;    /* Unlock watchdog 			*/
  WDOG->TOVAL	=0x0000FFFF;   	/* Maximum timeout value 	*/
  WDOG->CS 		=0x00002100;    /* Disable watchdog 		*/
}
int main(void)
{

	WDOG_disable();			/* Disable WDOG */
	Button_Init();			/* Configure ports */
	LED_Init();				/* Configure ports */
	SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NVIC_init_IRQs();       /* Enable desired interrupts and priorities */
	LPIT0_init();           /* Initialize PIT0 for 1 second timeout  */
	PTD-> PCOR |= 1<<PTD15;
    for (;;)
    {
    	BTN_Control_Led();
		if(TT_Buttton == 1)
		{
			__Blink_Led_ON(15);
			Delay_ms(1000);					//delay 1s
			__Blink_Led_OFF(15);
			__Blink_Led_ON(16);
			Delay_ms(1000);					//delay 1s
			__Blink_Led_OFF(16);
			__Blink_Led_ON(0);
			Delay_ms(1000);					//delay 1s
			__Blink_Led_OFF(0);
		}
    }
}
void PORTC_IRQHandler(void)
{
	if(PTC->PDIR & (1<<PTC13))
	{
		Delay_ms(20);
		if(PTC->PDIR & (1<<PTC13))
		{
			LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;  /* T_EN=1: Timer channel is enabled */
			while(PTC->PDIR & (1<<PTC13));
		}
	}
	else
	{
		LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;  /* T_EN=1: Timer channel is disabled */
	}
}
void SysTick_Handler(void)
{

}
void LPIT0_Ch0_IRQHandler(void)
{
	time++;							// every 1 ms increase up 1 
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}
void Delay_ms(unsigned int delay_ms)
{
	time = 0;
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;  /* T_EN=1: Timer channel is enabled */
	while(time < delay_ms);
	
	LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK; /* T_EN=0: Timer channel is disabled */

}
