#include <string.h>
#include "nrf_gpio.h"
#include "custom_board.h"
#include "shoponair.h"


#define SIZE_SOUND_BUFFER 16
static unsigned char sound_power_up = 0;

static unsigned char sound_data_buffer[SIZE_SOUND_BUFFER];
static unsigned char sound_q_front;
static unsigned char sound_q_rear;

extern bool print_pass_flag; //It from func.c 0209 KSD.MODIFY

static void play_sound_power_up(void)
{
extern void SoundPU(void);
    if(sound_power_up == 0){
        SoundPU();
        sound_power_up = 1;
    }
}

static void play_sound_power_down(void)
{
    if(sound_power_up){
        sound_power_up = 0;
    }
}

void play_sound_push(unsigned char data)
{
    sound_data_buffer[sound_q_front++] = data;
    if(sound_q_front >= 16)
        sound_q_front = 0;
}

void init_play_sound(void)
{   
    sound_q_front = sound_q_rear = 0;
    sound_power_up = 0;
}

unsigned char play_sound(void)
{
    unsigned char data;
    if(nrf_gpio_pin_read(DEF_AUDIO_BUSY) != 0)
        return 0;
    if(sound_q_front != sound_q_rear){
        data = sound_data_buffer[sound_q_rear++];
        if(sound_q_rear >= SIZE_SOUND_BUFFER)
            sound_q_rear = 0;
        play_sound_power_up();
        SoundPlay(data);
        return 1;
    }
    else{
        play_sound_power_down();
        return 0;
    }
}

void play_sound_set_total_value(unsigned char *total , _Bool last_sound)
{
    unsigned char num_of_digits;
    unsigned char offset_digits;
    unsigned char i;
    unsigned char proc_num = 0;
    num_of_digits = offset_digits = strlen((const char *)total);
    for( ; num_of_digits > 0; num_of_digits--){
        i = offset_digits - num_of_digits;
        switch(num_of_digits){
        case 7: 
            if(total[i] != '0')            proc_num++;
            if ((total[i + 1] == '0') && (total[i + 2] == '0')){
                if (total[i] == '1')      play_sound_push(54);  // "백만"
                else if (total[i] == '2') play_sound_push(55); // "이백만"
                else if (total[i] == '3') play_sound_push(56); // "삼백만"
                else if (total[i] == '4') play_sound_push(57); // "사백만"
                else if (total[i] == '5') play_sound_push(58); // "오백만"
                else if (total[i] == '6') play_sound_push(59); // "육백만"
                else if (total[i] == '7') play_sound_push(60); // "칠백만"
                else if (total[i] == '8') play_sound_push(61); // "팔백만"
                else if (total[i] == '9') play_sound_push(62); // "구백만"
            }
            else {
                if (total[i] == '1')       play_sound_push(18); // "백"
                else if (total[i] == '2') play_sound_push(19); // "이백"
                else if (total[i] == '3') play_sound_push(20); // "삼백"
                else if (total[i] == '4') play_sound_push(21); // "사백"
                else if (total[i] == '5') play_sound_push(22); // "오백"
                else if (total[i] == '6') play_sound_push(23); // "육백"
                else if (total[i] == '7') play_sound_push(24); // "칠백"
                else if (total[i] == '8') play_sound_push(25); // "팔백"
                else if (total[i] == '9') play_sound_push(26); // "구백"
            }
            break;
        case 6:
            if(total[i] != '0')            proc_num++;
            if (total[i + 1] == '0'){
                if (total[i] == '1')       play_sound_push(45); // "십만"
                else if (total[i] == '2') play_sound_push(46); // "이십만"
                else if (total[i] == '3') play_sound_push(47); // "삼십만"
                else if (total[i] == '4') play_sound_push(48); // "사십만"
                else if (total[i] == '5') play_sound_push(49); // "오십만"
                else if (total[i] == '6') play_sound_push(50); // "육십만"
                else if (total[i] == '7') play_sound_push(51); // "칠십만"
                else if (total[i] == '8') play_sound_push(52); // "팔십만"
                else if (total[i] == '9') play_sound_push(53); // "구십만"
            }
            else {
                if (total[i] == '1')       play_sound_push(9); // "십"
                else if (total[i] == '2')  play_sound_push(10); // "이십"
                else if (total[i] == '3')  play_sound_push(11); // "삼십"
                else if (total[i] == '4')  play_sound_push(12); // "사십"
                else if (total[i] == '5')  play_sound_push(13); // "오십"
                else if (total[i] == '6')  play_sound_push(14); // "육십"
                else if (total[i] == '7')  play_sound_push(15); // "칠십"
                else if (total[i] == '8')  play_sound_push(16); // "팔십"
                else if (total[i] == '9')  play_sound_push(17); // "구십"
            }
            break;
        case 5:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1') // 예시) "110,000" 십일만원으로 출력해야하는 조건
            {
                play_sound_push(36);
            }
            else if (total[i] == '2')      play_sound_push(37); // "이만"
            else if (total[i] == '3')      play_sound_push(38); // "삼만"
            else if (total[i] == '4')      play_sound_push(39); // "사만"
            else if (total[i] == '5')      play_sound_push(40); // "오만"
            else if (total[i] == '6')      play_sound_push(41); // "육만"
            else if (total[i] == '7')      play_sound_push(42); // "칠만"
            else if (total[i] == '8')      play_sound_push(43); // "팔만"
            else if (total[i] == '9')      play_sound_push(44); // "구만"
            break;
        case 4:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(27); // "천"
            else if (total[i] == '2')      play_sound_push(28); // "이천"
            else if (total[i] == '3')      play_sound_push(29); // "삼천"
            else if (total[i] == '4')      play_sound_push(30); // "사천"
            else if (total[i] == '5')      play_sound_push(31); // "오천"
            else if (total[i] == '6')      play_sound_push(32); // "육천"
            else if (total[i] == '7')      play_sound_push(33); // "칠천"
            else if (total[i] == '8')      play_sound_push(34); // "팔천"
            else if (total[i] == '9')      play_sound_push(35); // "구천"
            break;
        case 3:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(18); // "백"
            else if (total[i] == '2')      play_sound_push(19); // "이백"
            else if (total[i] == '3')      play_sound_push(20); // "삼백"
            else if (total[i] == '4')      play_sound_push(21); // "사백"
            else if (total[i] == '5')      play_sound_push(22); // "오백"
            else if (total[i] == '6')      play_sound_push(23); // "육백"
            else if (total[i] == '7')      play_sound_push(24); // "칠백"
            else if (total[i] == '8')      play_sound_push(25); // "팔백"
            else if (total[i] == '9')      play_sound_push(26); // "구백"
            break;
        case 2:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(9); // "십"
            else if (total[i] == '2')      play_sound_push(10); // "이십"
            else if (total[i] == '3')      play_sound_push(11); // "삼십"
            else if (total[i] == '4')      play_sound_push(12); // "사십"
            else if (total[i] == '5')      play_sound_push(13); // "오십"
            else if (total[i] == '6')      play_sound_push(14); // "육십"
            else if (total[i] == '7')      play_sound_push(15); // "칠십"
            else if (total[i] == '8')      play_sound_push(16); // "팔십"
            else if (total[i] == '9')      play_sound_push(17); // "구십"
            break;
        case 1:
            if (total[i] == '1')            play_sound_push(0); //"일원"
            else if (total[i] == '2')      play_sound_push(1); //"이원"
            else if (total[i] == '3')      play_sound_push(2); //"삼원"
            else if (total[i] == '4')      play_sound_push(3); //"사원"
            else if (total[i] == '5')      play_sound_push(4); //"오원"
            else if (total[i] == '6')      play_sound_push(5); //"육원"
            else if (total[i] == '7')      play_sound_push(6); //"칠원"
            else if (total[i] == '8')      play_sound_push(7); //"팔원"
            else if (total[i] == '9')      play_sound_push(8); //"구원"
            else if (total[i] == '0'){
                if(proc_num != 0)
                    play_sound_push(63); //"원"
            }
            break;
        case 0:
            break;
        default:
            break;
        }
    }
    //BLE Auto cut off 기능추가 2020.02.07 ksd
    //CFG_MODE_KEYPAD 모드에서는 금액 파싱한다음 결제되었습니다. 음성을 미출력 하는 조건 추가
    if (last_sound)
    {
        play_sound_push(77); //"결제되었습니다"
		
        if (print_pass_flag)
        {
           PRINT_PASS_SOUND; //"인쇄를생략합니다"
        }
    }
}
