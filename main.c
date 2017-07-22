#include "main.h"
#include <stdio.h>
typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;
uint32_t TPD_Speed[4]={650,950,1250,1850};
uint16_t TPD_Delay[4]={798,546,415,280};
MCH_Typedef MCH_Set[4];//luu tru cau hinh
Motor_Typedef Mo[MOTOR_NUM];
uint8_t mNum=0;
uint8_t minute=0,second=0;

#define TxBufferSize   (countof(TxBuffer)-1)
#define RxBufferSize   10

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))
/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
char RxBuffer[RxBufferSize+1];
__IO uint8_t RxCounter = 0x00; 
uint8_t NbrOfDataToRead = RxBufferSize;
__IO TestStatus TransferStatus = FAILED; 
uint8_t mLine=0,mArea=0,mDir=0,mMCH=0;
uint8_t mTPD=0;

ErrorStatus  HSEStartUpStatus;
FLASH_Status FlashStatus;
uint16_t VarValue = 0;
uint16_t khoangcach=0;
/* Virtual address defined by the user: 0xFFFF value is prohibited */

uint16_t data[10],dataRead[10];
uint16_t dataTab[4];

uint8_t str2[20];
uint8_t srf_buffer[20],srf_pow[MOTOR_NUM];
uint8_t srf_num=0,srf_count=0,srf_run=1;
uint8_t srf_t =0;
uint32_t srf_i=0;
void Motor_Init(void);
int main(){	  
	uint8_t i,j=0;
	//SetSysClockTo24();
	SystemCoreClockUpdate();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  GPIO_Configuration();
  NVIC_Configuration(); 
	EXTI_Configuration();
	TIMER1_Configuration();
	TIMER2_Configuration();
	TIMER3_Configuration();
	TIMER4_Configuration();
	USART1_Configuration();
	USART2_Configuration();
  /* Unlock the Flash Program Erase controller */
//  FLASH_Unlock();

//  /* EEPROM Init */
  EE_Init();
  Motor_Init();
	delay_init();
  delay_ms(1000);
  lcd1621_init();
  sendcmd(bias);
  sendcmd(system);
  sendcmd(lcdon); 
  lcd_clear();
	RxInit();
  
  while(1){
		srf_i++;
		if(srf_i>5){
			srf_i=0;
		khoangcach=GetDistance();
		srf_buffer[srf_num]=khoangcach;
		srf_num++;
			srtf_t++;
		if(srf_num>=5){
			srf_num=0;
			for(i=0;i<5;i++){
				if(srf_buffer[0]-srf_buffer[i]<=1)
					srf_count++;
			}
			if(srf_count==5){
				srf_run=0;				
			}
			else{
				srf_run=1;
			}
			srf_count=0;
		}
		
		if(srf_run==0){
			for(i=0;i<MOTOR_NUM;i++){
				srf_pow[i]=Mo[i].Pow;
				Mo[i].Pow=SLEEP;				
			}			
			sprintf(str2,"[%d]motor sleep: %d cm\n",srf_t,khoangcach);
		}
		else{
			for(i=0;i<MOTOR_NUM;i++)
				Mo[i].Pow=srf_pow[i];
			sprintf(str2,"[%d]motor run: %d cm\n",srf_t,khoangcach);
		}		
	}
		USART_SendString(USART1,str2);
		main_processing();		
		print_screen(mArea+1,mLine+1,Mo[mLine].MCH+1,TPD_Speed[Mo[mLine].Speed],Mo[mLine].Di,Mo[mLine].Pow);			
  }
}
void Motor_Init(){
	uint8_t i=0,j=0;
	for(i=0;i<4;i++){
		MCH_Set[i].Di=RIGHT;
		MCH_Set[i].Speed=mTPD;
	}
  for(i=0;i<MOTOR_NUM;i++){
		for(j=0;j<4;j++)
			EE_ReadVariable(i*4+j,&dataTab[j]);
		if(dataTab[0]==0xFF){
			dataTab[0]=mMCH;
			EE_WriteVariable(i*4,mMCH);			
		}
		if(dataTab[1]==0xFF){
			dataTab[1]=MCH_Set[mMCH].Speed;
			EE_WriteVariable(i*4+1,dataTab[1]);
		}
		if(dataTab[2]==0xFF){
			dataTab[2]=MCH_Set[mMCH].Di;
			EE_WriteVariable(i*4+2,dataTab[2]);
		}
		if(dataTab[3]==0xFF){
			dataTab[3]=SLEEP;
			EE_WriteVariable(i*4+3,dataTab[3]);
		}		
		Mo[i].Pow=dataTab[3];
		Mo[i].Di=dataTab[2];
		Mo[i].Speed=dataTab[1];
		Mo[i].MCH=dataTab[0];
		srf_pow[i]=Mo[i].Pow;
  }
	//USART_SendString(USART2,"reset");
}


PUTCHAR_PROTOTYPE
{	
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);
  
  //  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}
  
  return ch;
}
#ifdef USE_FULL_ASSERT
void assert_failed(*uint8_t file, uint32_t line){
  while(1);
}
#endif
