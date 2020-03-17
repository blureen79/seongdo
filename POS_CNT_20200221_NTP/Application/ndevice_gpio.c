#include "custom_board.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "ndevice_gpio.h"
#include "ndevice_pwm.h"

void nd_gpio_init(void)
{
    //Initialize GPIOs
    nrf_gpio_cfg_output(DEF_GPIO_RED1_PWM);
    nrf_gpio_cfg_output(DEF_GPIO_SMDLED_D1);
    nrf_gpio_cfg_output(DEF_GPIO_AUDIOSHDN);
    //nrf_gpio_cfg_output(DEF_AUDIO_RESET);
    nrf_gpio_cfg_output(DEF_SOUND_SDO);
    nrf_gpio_cfg_output(DEF_SOUND_SCK);
    nrf_gpio_cfg_output(CAT2_PIN_NUMBER);
    nrf_gpio_cfg_output(TX_PIN_NUMBER);
    nrf_gpio_pin_clear(TX_PIN_NUMBER);
   
    nrf_gpio_cfg_output(DEF_SPI_SRAM_HOLD);
    nrf_gpio_pin_set(DEF_SPI_SRAM_HOLD);

    nrf_gpio_cfg_input(DEF_GPIO_BUTTON,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(21U, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(DEF_AUDIO_BUSY, NRF_GPIO_PIN_NOPULL);

    nrf_gpio_pin_set(DEF_GPIO_RED1_PWM);
    nrf_gpio_pin_set(DEF_GPIO_SMDLED_D1);
    nrf_gpio_pin_set(DEF_GPIO_AUDIOSHDN);
    
    //nrf_gpio_pin_set(DEF_AUDIO_RESET);
    nrf_gpio_pin_set(DEF_SOUND_SDO);
    nrf_gpio_pin_set(DEF_SOUND_SCK);

    //onepeace program error 발생하여 CAT2_PIN_NUMBER 단자를 high 상태로 세팅
    nrf_gpio_pin_set(CAT2_PIN_NUMBER);

    //ADG1436 Switch IC Driection Select
    nrf_gpio_cfg_output(RS232_IN1);
    nrf_gpio_cfg_output(RS232_IN2);
    nrf_gpio_cfg_output(POS_DIR);
   
    nrf_gpio_pin_set(RS232_IN1);
    nrf_gpio_pin_clear(POS_DIR);
		
		//HC595 Write Pin state
		nrf_gpio_cfg_output(CL1);
		nrf_gpio_cfg_output(CL2);
		nrf_gpio_cfg_output(SHIFT_DATA);
		nrf_gpio_cfg_output(SHIFT_CLOCK);
		nrf_gpio_cfg_output(LATCH_CLOCK);
		
		nrf_gpio_pin_clear(CL1);
		nrf_gpio_pin_clear(CL2);

}
