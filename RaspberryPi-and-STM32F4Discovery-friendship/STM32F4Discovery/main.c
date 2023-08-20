// EXAMPLE of firmware to connect STM32F4Discovery to Raspberry Pi
// Provided for educational needs for DMZCON-2023, international cyber security conference workshops

#include "stm32f4xx_hal.h"
#define MAXPINCOUNT	64
#define ANTIDREBEZG_TIMEOUT	2

#define STABLE_MAX							100


//////////////////////////////////////////
#define INPUT_FIRST_INDEX				1

#define	I_INPUT_1			1
#define	I_INPUT_2			2
// ..........
#define	I_INPUT_28				28
#define INPUT_LAST_INDEX				28

////////////////////////////////////////////////////

#define OUTPUT_FIRST_INDEX			29

#define	O_OUTPUT_1			29
#define	O_OUTPUT_2			30
// ............
#define	O_OUTPUT_N			53
#define OUTPUT_LAST_INDEX				53


static unsigned int pinarr[MAXPINCOUNT] = {0x00};
static unsigned int clickedarr[MAXPINCOUNT] = {0x00};
static unsigned int stable_counter_arr[MAXPINCOUNT] = {0x00};
static unsigned int prevpinarr[MAXPINCOUNT] = {0x00};
static unsigned int pinmodearr[MAXPINCOUNT] = {0x00};
static uint16_t indexarr[MAXPINCOUNT] = {0x00};
static unsigned int tdarr[MAXPINCOUNT];


void Init_OnBoard_LEDs(void);
void SetIndexes(void);
void Delay_ms(volatile int time_ms);
int main(void)
{
	
	unsigned int write_counter = 0, read_counter = 0;
	unsigned int i = 0;
	GPIO_PinState read_bit;
	GPIO_PinState clk_state = 0;
	GPIO_PinState clk_antidrebezg = 0;
	unsigned int antidrebezg_counter = 0;
	GPIO_PinState clk_prev_state = 0;
	unsigned int write_bit;
	GPIO_PinState temp_state = 0;
	GPIO_PinState PrevEnableMode = 0, IsConnectedMode = 0;

	// INITIALIZATION	
	Init_OnBoard_LEDs();
	SetIndexes();

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);  // EXAMPLE
	
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);	 // Set RECV-DATA PIN to ON - system is ready to connect with raspberry

	for(i=0;i<MAXPINCOUNT;i++) {
		prevpinarr[i] = 0;
		pinarr[i] = 0;
		stable_counter_arr[i] = 0;
	}
	
	while(1) {

		// UPDATE PINs STATE
		
		////////// OUTPUTS: set array values to PINs ////////////////
		
		for(i=OUTPUT_FIRST_INDEX;i<=OUTPUT_LAST_INDEX;i++) {
			if(prevpinarr[i] != pinarr[i]) {
				if(pinarr[i] == 0) {
					switch(tdarr[i]) {  // SETING ZERO TO PIN
						case 0:
							HAL_GPIO_WritePin(GPIOA,indexarr[i],GPIO_PIN_RESET);break;
						case 1:
							HAL_GPIO_WritePin(GPIOB,indexarr[i],GPIO_PIN_RESET);break;
						case 2:
							HAL_GPIO_WritePin(GPIOC,indexarr[i],GPIO_PIN_RESET);break;
						case 3:
							HAL_GPIO_WritePin(GPIOD,indexarr[i],GPIO_PIN_RESET);break;
						case 4:
							HAL_GPIO_WritePin(GPIOE,indexarr[i],GPIO_PIN_RESET);break;
						default:
							// TODO: add exception handle here
							//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
						break;
					}
			  } else { // SETTING V TO PIN
					switch(tdarr[i]) {
						case 0:
							HAL_GPIO_WritePin(GPIOA,indexarr[i],GPIO_PIN_SET);break;
						case 1:
							HAL_GPIO_WritePin(GPIOB,indexarr[i],GPIO_PIN_SET);break;
						case 2:
							HAL_GPIO_WritePin(GPIOC,indexarr[i],GPIO_PIN_SET);break;
						case 3:
							HAL_GPIO_WritePin(GPIOD,indexarr[i],GPIO_PIN_SET);break;
						case 4:
							HAL_GPIO_WritePin(GPIOE,indexarr[i],GPIO_PIN_SET);break;				
						default:
							// TODO: add exception handle here
							// HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
						break;
					}
				}
			}
    }
		
		////////////////// INPUTS: get values from PINs and put data to the Array /////////
		
    for(i=1;i<29;i++) {
				temp_state = 0;
				switch(tdarr[i]) {
					case 0:
						temp_state = HAL_GPIO_ReadPin(GPIOA,indexarr[i]); break;
					case 1:
						temp_state = HAL_GPIO_ReadPin(GPIOB,indexarr[i]); break;
					case 2:
						temp_state = HAL_GPIO_ReadPin(GPIOC,indexarr[i]); break;
					case 3:
						temp_state = HAL_GPIO_ReadPin(GPIOD,indexarr[i]); break;
					case 4:
						temp_state = HAL_GPIO_ReadPin(GPIOE,indexarr[i]); break;
					default:
							// TODO: add exception handle here
					break;
				}
			
				if((prevpinarr[i] != 0 && temp_state == 0) || (prevpinarr[i] == 0 && temp_state != 0)) {
           stable_counter_arr[i] = 0;
					 if(temp_state == 0) {
						  prevpinarr[i] = 0;
					 } else {
						  prevpinarr[i] = 1;
					 }
					 } else {
					 stable_counter_arr[i] = (stable_counter_arr[i]>=STABLE_MAX)?STABLE_MAX:stable_counter_arr[i]+1;
				}
				if(stable_counter_arr[i] >= STABLE_MAX) {
					if(temp_state == 0) {
						pinarr[i] = 0;
						prevpinarr[i] = 0;
						stable_counter_arr[i] = 0;
					} else {
						pinarr[i] = 1;
						prevpinarr[i] = 1;
						clickedarr[i] = 1;
						stable_counter_arr[i] = 0;
					}
				}
		}

//// PIN & ARRAY UPDATE FINISHED		
		
////////////////////////////////		
		// LETS PLACE SOME LOGICS AT THE STM32 SIDE ALSO:
		
		IsConnectedMode = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6);
		
		if(IsConnectedMode == 0) {  // AUTONOMOUS MODE (WITHOUT RASPBERRY PI)
				// Setting CONNECTION-ERROR-MODE (EXCHANGE-COMPLETED-MODE):  Set 1-> RECV_DATA_PIN
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET);  // Sending the CLK_ACK reply - 0
				PrevEnableMode = GPIO_PIN_RESET;
		
			// TODO: find out, if drebezg happens for IsConnectedMode value			

///////////// AUTONOMOUS //////////////////////////////////////////
/////// SOME AUTONOMOUS LOGICS HERE ////
				
				for(i=29;i<49;i++) {  // SAVE previous PIN state in PREVPINARR
					prevpinarr[i] = pinarr[i];
				}
				
// EXAMPLE:				
				pinarr[O_OUTPUT_N] = pinarr[I_INPUT_2];
			
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
		
		} else {   // RASPBERRY-PI CONNECTED MODE (MANAGED MODE)
				if(PrevEnableMode == 0) { // Just connected
							write_counter = 0;
							clk_prev_state = 0;
							read_counter = 0;
							clk_antidrebezg = 0;
							antidrebezg_counter = 0;
					// RESetting CONNECTION-ERROR-MODE (EXCHANGE-COMPLETED-MODE):  Set 0-> RECV_DATA_PIN
							HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET);  // Sending the CLK_ACK reply - 0
				}
				PrevEnableMode = IsConnectedMode;
		
		
		// CLK - CLK_ACK
				clk_state = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7);  // READ the CLK from RASPBERRY
				
				if(clk_state != clk_antidrebezg) {
  					antidrebezg_counter=0;
				} else {
		  			antidrebezg_counter=(antidrebezg_counter>=ANTIDREBEZG_TIMEOUT)?ANTIDREBEZG_TIMEOUT:antidrebezg_counter+1;
				}
 				clk_antidrebezg = clk_state; // Save current state to test next signal
				
				if(antidrebezg_counter >= ANTIDREBEZG_TIMEOUT) {  // Current state is stable enough to trust it
					antidrebezg_counter=0;
				  if(clk_antidrebezg != clk_prev_state) {  // CLK toggled/changed its state
							if(clk_antidrebezg != GPIO_PIN_RESET) { // if FRONT of the CLK
									if(read_counter <64) {  // First we read data from raspberry, then we write data to raspberry
											HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
										  read_bit = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9);
										//////////////////// SETTING PIN-STATE FROM RASPBERRY ///////////////////////////
											prevpinarr[read_counter] = pinarr[read_counter]; // save previous state to check if the state was changed (during the PIN-update)
											if(read_bit == 0) 
													pinarr[read_counter] = 0x00;
											else
													pinarr[read_counter] = 1;
										///////////////////////////////////////////////////////////////////////////////////
											read_counter ++;
											write_counter = 0;
									} else { // read completed - begin write
											if(write_counter < 64) {
													write_bit = pinarr[write_counter];
													if(write_bit == 0x00)
															HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);
													else
															HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_SET);
													write_counter++;
											} else { // write completed - finish exchange circle and start new circle

										// Set the FLAG, that the exchange completed successfully
												  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
													read_counter = 0;
													write_counter = 0;
													clk_prev_state = 0;
													clk_antidrebezg = 0;
											}
									}
							}
							if(clk_antidrebezg == GPIO_PIN_RESET) {
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET);  // Sending the CLK_ACK reply - 0
							} else {
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_SET);  // Sending the CLK_ACK reply - 1
							}
					} else {
// Do nothing here and wait for CLK change from Raspberry						

/////////////////////////////
					}
					
    			clk_prev_state = clk_antidrebezg;
				  antidrebezg_counter = 0;
			 }

	  }  // RASPBERRY CONNECTED block finish

		Delay_ms(1);

	}
	
	return 0;
}

////////////  BE VERY ACCURATE WHEN EDITING THIS FUNCTION :)))

void Init_OnBoard_LEDs(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	GPIO_InitTypeDef BoardOutputsA;
	GPIO_InitTypeDef BoardOutputsB;
	GPIO_InitTypeDef BoardOutputsC;
//	GPIO_InitTypeDef BoardOutputsD;
	GPIO_InitTypeDef BoardOutputsE;
	
// GPIO_InitTypeDef BoardInputsA;
	GPIO_InitTypeDef BoardInputsB;
	GPIO_InitTypeDef BoardInputsC;
	GPIO_InitTypeDef BoardInputsD;
	GPIO_InitTypeDef BoardInputsE;

////////////////// OUTPUT INITIALIZATION EXAMPLE //////////////////
	
	BoardOutputsA.Mode = GPIO_MODE_OUTPUT_PP;
  BoardOutputsA.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8; // DONT USE: | GPIO_PIN_13;
	HAL_GPIO_Init(GPIOA, &BoardOutputsA);

	BoardOutputsB.Mode = GPIO_MODE_OUTPUT_PP;
	BoardOutputsB.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	HAL_GPIO_Init(GPIOB, &BoardOutputsB);

	BoardOutputsC.Mode = GPIO_MODE_OUTPUT_PP;
//	BoardOutputsC.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	BoardOutputsC.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8;
	HAL_GPIO_Init(GPIOC, &BoardOutputsC);
	
//	BoardOutputsD.Mode = GPIO_MODE_OUTPUT_PP;
//	BoardOutputsD.Pin = GPIO_PIN_0;
//	HAL_GPIO_Init(GPIOD, &BoardOutputsD);

	BoardOutputsE.Mode = GPIO_MODE_OUTPUT_PP;
	BoardOutputsE.Pin = GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &BoardOutputsE);

///////////////////// INPUT INITIALIZATION EXAMPLE ///////////////

	BoardInputsB.Mode = GPIO_MODE_INPUT;
	BoardInputsB.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;// | GPIO_PIN_9;
	BoardInputsB.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &BoardInputsB);


	BoardInputsC.Mode = GPIO_MODE_INPUT;
	BoardInputsC.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	BoardInputsC.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &BoardInputsC);

  BoardInputsD.Mode = GPIO_MODE_INPUT;
	BoardInputsD.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7;// | GPIO_PIN_13 | GPIO_PIN_14;
	BoardInputsD.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOD, &BoardInputsD);

  BoardInputsE.Mode = GPIO_MODE_INPUT;
	BoardInputsE.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3  | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	BoardInputsE.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &BoardInputsE);

}

void SetIndexes(void) {
	// OUTPUTS EXAMPLES
	indexarr[O_OUTPUT_1] = GPIO_PIN_0;
	tdarr[O_OUTPUT_1] = 2;
	pinmodearr[O_OUTPUT_1] = 1;
	
	// INPUTS EXAMPLES
	indexarr[I_INPUT_1] = GPIO_PIN_12; // e12 pin
	tdarr[I_INPUT_1] = 4;
	indexarr[I_INPUT_2] = GPIO_PIN_11; // e11 pin
	tdarr[I_INPUT_2] = 4;	

}

void Delay_ms(volatile int time_ms) {
	      int j;
        for(j = 0; j < time_ms*10; j++)
            {}  
}
