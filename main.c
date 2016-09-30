/*

Copyright Cynthia Thamires

This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
*/

#include "18F452.h"
#include <stdio.h>
#include <stdlib.h>
#use delay(clock = 20000000)
#fuses HS, NOPUT, NOWDT, NOBROWNOUT, NOLVP, NODEBUG

#define falha pin_c5
#define pwm1 pin_c2
#define pwm2 pin_c1

unsigned long int dente = 0;
int flag = 0;
int sincronismo = 0;
long int t;
long int tref;
long int soma = 15;
long int conta = 0;
long int dentefalha = 0;

// Matriz para a realização da rampa de corrente.

unsigned int byCurrentTable [16] =
{0x3f,0x3f,0x3f,0x3f,0x3f,0x1f,0x1f,0x1f,0x1f,0x0f,0x0f,0x07,0x07,0x07,0x07,0x07};

#int_timer1
void trata_timer1 (void)
{
   conta ++;
   set_timer1 (60535 + get_timer1());
}

#int_ext
void trataRB0 (void)
{

   dente ++; //Incrementa a variável dente a cada interrupção externa




 // O algoritmo abaixo realiza a identificação da falha, comparando o período dos dentes
 // e no momento em que existe um período maior do que o anterior, é detectada a falha.

   dentefalha ++;

if (sincronismo == 5 && dente == 50)
   {
      flag = 0;
      sincronismo = 0;
   }

   switch (flag)

   {
      case 3:
      output_low(falha);

      if (dente >= 70)
      {
         output_high (falha);
         dente = 0;
         dentefalha = 0;
         sincronismo ++;
      }
      break;

      case 2:
      t = conta;
             if (t <= tref)
            {
               tref = (t * soma)/10;
               conta = 0;
               flag = 2;
            }

             if (t > tref)
            {
               output_high(falha);
               flag = 3;
               dente = 0;
               dentefalha = 0;
            }
       break;


  case 1:
       t = conta;
       tref = (t * soma)/10;
       conta = 0;
       flag = 2;
       break;

       case 0:
       conta = 0;
       flag = 1;
       output_low(falha);
       break;

   }

   //o algoritmo abaixo executa o acionamento do solenoide emulando uma
   //rampa decorrente decrescente de 3 degraus de modo a reduzir o aquecimento
   //exemplo:         I(t)^
   //            75%      |  --\
   //            50%      |      --\
   //            25%      |          --\____
   //                     ---------------------> t(s)

            if((dente <= 25)&&(dente > 9)) //escape
            {
               set_pwm2_duty(byCurrentTable[dente-10]);
            }
            if (dente > 25)
            {
                set_pwm2_duty (0x00);
            }

            if((dente <= 42)&&(dente > 26)) //admissão
            {
               set_pwm1_duty(byCurrentTable[dente-27]);
            }
            if (dente > 42)
            {
                set_pwm1_duty (0x00);
            }

}



void main()
{

   setup_adc(ADC_OFF);
   setup_psp(PSP_DISABLED);
   setup_spi(SPI_SS_DISABLED);
   setup_wdt(WDT_OFF);
   setup_timer_0(RTCC_INTERNAL);
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);
   setup_ccp1(CCP_PWM);
   setup_ccp2(CCP_PWM);
   set_pwm1_duty(0x00);
   set_pwm2_duty(0x00);
   setup_timer_2(T2_DIV_BY_1,50,1);//modulo PWM operando 100KHz com 6bits
   disable_interrupts(GLOBAL);
   delay_ms(1);
   set_tris_a(0xFF);
   set_tris_b(0xFF);
   set_tris_c(0x00);
   set_tris_d(0x00);
   enable_interrupts(GLOBAL);
   delay_us(200);
   enable_interrupts(INT_TIMER1);
   enable_interrupts(INT_EXT);
   ext_int_edge(H_TO_L);

   while (true);
}
