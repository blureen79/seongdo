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
                if (total[i] == '1')      play_sound_push(54);  // "�鸸"
                else if (total[i] == '2') play_sound_push(55); // "�̹鸸"
                else if (total[i] == '3') play_sound_push(56); // "��鸸"
                else if (total[i] == '4') play_sound_push(57); // "��鸸"
                else if (total[i] == '5') play_sound_push(58); // "���鸸"
                else if (total[i] == '6') play_sound_push(59); // "���鸸"
                else if (total[i] == '7') play_sound_push(60); // "ĥ�鸸"
                else if (total[i] == '8') play_sound_push(61); // "�ȹ鸸"
                else if (total[i] == '9') play_sound_push(62); // "���鸸"
            }
            else {
                if (total[i] == '1')       play_sound_push(18); // "��"
                else if (total[i] == '2') play_sound_push(19); // "�̹�"
                else if (total[i] == '3') play_sound_push(20); // "���"
                else if (total[i] == '4') play_sound_push(21); // "���"
                else if (total[i] == '5') play_sound_push(22); // "����"
                else if (total[i] == '6') play_sound_push(23); // "����"
                else if (total[i] == '7') play_sound_push(24); // "ĥ��"
                else if (total[i] == '8') play_sound_push(25); // "�ȹ�"
                else if (total[i] == '9') play_sound_push(26); // "����"
            }
            break;
        case 6:
            if(total[i] != '0')            proc_num++;
            if (total[i + 1] == '0'){
                if (total[i] == '1')       play_sound_push(45); // "�ʸ�"
                else if (total[i] == '2') play_sound_push(46); // "�̽ʸ�"
                else if (total[i] == '3') play_sound_push(47); // "��ʸ�"
                else if (total[i] == '4') play_sound_push(48); // "��ʸ�"
                else if (total[i] == '5') play_sound_push(49); // "���ʸ�"
                else if (total[i] == '6') play_sound_push(50); // "���ʸ�"
                else if (total[i] == '7') play_sound_push(51); // "ĥ�ʸ�"
                else if (total[i] == '8') play_sound_push(52); // "�Ƚʸ�"
                else if (total[i] == '9') play_sound_push(53); // "���ʸ�"
            }
            else {
                if (total[i] == '1')       play_sound_push(9); // "��"
                else if (total[i] == '2')  play_sound_push(10); // "�̽�"
                else if (total[i] == '3')  play_sound_push(11); // "���"
                else if (total[i] == '4')  play_sound_push(12); // "���"
                else if (total[i] == '5')  play_sound_push(13); // "����"
                else if (total[i] == '6')  play_sound_push(14); // "����"
                else if (total[i] == '7')  play_sound_push(15); // "ĥ��"
                else if (total[i] == '8')  play_sound_push(16); // "�Ƚ�"
                else if (total[i] == '9')  play_sound_push(17); // "����"
            }
            break;
        case 5:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1') // ����) "110,000" ���ϸ������� ����ؾ��ϴ� ����
            {
                play_sound_push(36);
            }
            else if (total[i] == '2')      play_sound_push(37); // "�̸�"
            else if (total[i] == '3')      play_sound_push(38); // "�︸"
            else if (total[i] == '4')      play_sound_push(39); // "�縸"
            else if (total[i] == '5')      play_sound_push(40); // "����"
            else if (total[i] == '6')      play_sound_push(41); // "����"
            else if (total[i] == '7')      play_sound_push(42); // "ĥ��"
            else if (total[i] == '8')      play_sound_push(43); // "�ȸ�"
            else if (total[i] == '9')      play_sound_push(44); // "����"
            break;
        case 4:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(27); // "õ"
            else if (total[i] == '2')      play_sound_push(28); // "��õ"
            else if (total[i] == '3')      play_sound_push(29); // "��õ"
            else if (total[i] == '4')      play_sound_push(30); // "��õ"
            else if (total[i] == '5')      play_sound_push(31); // "��õ"
            else if (total[i] == '6')      play_sound_push(32); // "��õ"
            else if (total[i] == '7')      play_sound_push(33); // "ĥõ"
            else if (total[i] == '8')      play_sound_push(34); // "��õ"
            else if (total[i] == '9')      play_sound_push(35); // "��õ"
            break;
        case 3:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(18); // "��"
            else if (total[i] == '2')      play_sound_push(19); // "�̹�"
            else if (total[i] == '3')      play_sound_push(20); // "���"
            else if (total[i] == '4')      play_sound_push(21); // "���"
            else if (total[i] == '5')      play_sound_push(22); // "����"
            else if (total[i] == '6')      play_sound_push(23); // "����"
            else if (total[i] == '7')      play_sound_push(24); // "ĥ��"
            else if (total[i] == '8')      play_sound_push(25); // "�ȹ�"
            else if (total[i] == '9')      play_sound_push(26); // "����"
            break;
        case 2:
            if(total[i] != '0')            proc_num++;
            if (total[i] == '1')            play_sound_push(9); // "��"
            else if (total[i] == '2')      play_sound_push(10); // "�̽�"
            else if (total[i] == '3')      play_sound_push(11); // "���"
            else if (total[i] == '4')      play_sound_push(12); // "���"
            else if (total[i] == '5')      play_sound_push(13); // "����"
            else if (total[i] == '6')      play_sound_push(14); // "����"
            else if (total[i] == '7')      play_sound_push(15); // "ĥ��"
            else if (total[i] == '8')      play_sound_push(16); // "�Ƚ�"
            else if (total[i] == '9')      play_sound_push(17); // "����"
            break;
        case 1:
            if (total[i] == '1')            play_sound_push(0); //"�Ͽ�"
            else if (total[i] == '2')      play_sound_push(1); //"�̿�"
            else if (total[i] == '3')      play_sound_push(2); //"���"
            else if (total[i] == '4')      play_sound_push(3); //"���"
            else if (total[i] == '5')      play_sound_push(4); //"����"
            else if (total[i] == '6')      play_sound_push(5); //"����"
            else if (total[i] == '7')      play_sound_push(6); //"ĥ��"
            else if (total[i] == '8')      play_sound_push(7); //"�ȿ�"
            else if (total[i] == '9')      play_sound_push(8); //"����"
            else if (total[i] == '0'){
                if(proc_num != 0)
                    play_sound_push(63); //"��"
            }
            break;
        case 0:
            break;
        default:
            break;
        }
    }
    //BLE Auto cut off ����߰� 2020.02.07 ksd
    //CFG_MODE_KEYPAD ��忡���� �ݾ� �Ľ��Ѵ��� �����Ǿ����ϴ�. ������ ����� �ϴ� ���� �߰�
    if (last_sound)
    {
        play_sound_push(77); //"�����Ǿ����ϴ�"
		
        if (print_pass_flag)
        {
           PRINT_PASS_SOUND; //"�μ⸦�����մϴ�"
        }
    }
}
