#include <Arduino.h>
#include "wiring_private.h" 
#include "CMPS14.h"
#define PIN_SPI_CS 4 //Pin di collegamento del sd
#define CMPS_GET_CALIBRATION_STATE 0x24 //PID per la ricezione dello stato di calibrazione
//Tutte le configurazioni per la calibrazione
#define CAL_MAG B10000001
#define CAL_ACC B10000010
#define CAL_GYR B10000100
#define CAL_STOP B10000000
//PID da noi usati
#define ROLL 0x26
#define PITCH 0x27 
#define ACC 0x28
#define LIN_ACC 0x20
// imuSerial pin and pad definitions (in Arduino files Variant.h & Variant.cpp) I2C
#define PIN_SERIAL2_RX       (20u)               // Pin description number for PIO_SERCOM on D32 SDA
#define PIN_SERIAL2_TX       (21u)               // Pin description number for PIO_SERCOM on D33 SCL
#define PAD_SERIAL2_TX       (UART_TX_PAD_0)      // SERCOM pad 0
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_1)    // SERCOM pad 1

// Instantiate the Serial2 class
Uart imuSerial(&sercom3, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
byte calibration_code[3] = {0x98, 0x95, 0x99}, ok_check; //byte necessari per attivare la ricalibrazione
byte storing [3] = {0xF0, 0xF5, 0xF6};//Byte necessari allo storage della calibrazione
int prova = 25;
char* value;
int8_t tmp1byte; //Temp byte al fine di effetturare il merge tra 2 byte
int16_t tmp2byte; //2 byte temporanei post merge
//SERCOM sercom5( SERCOM5 ) ;
//{ PORTA, 22, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_6 }; // SDA: SERCOM3/PAD[0]
//{ PORTA, 23, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_7 };

//Costruttore, qui stiamo prendendo in ingresso un puntatore alla softwareserial creato nel main
void SERCOM4_Handler(){
  imuSerial.IrqHandler();
}

CMPS14::CMPS14(){  


}
/*
  Start della seriale
*/
void CMPS14::BeginSerial(int baud){
   imuSerial.begin(baud);

}
// Riprogrammazione dei pin seriali, può esser fatto anche nel main
void CMPS14::setSerial(int TX, int RX){
   //
}
//Calibrazione
int CMPS14::Calibration(char type){
     for (int i = 0; i < 3; i++) {
        //Serial.println("Start :");
        imuSerial.write(calibration_code[i]);
        ok_check = imuSerial.read();
        if (ok_check == 0x55) {
          switch(type){
              case 'a':
                //Serial.println("Accel");
                imuSerial.write(CAL_ACC);
                break;
              case 'g':
                //Serial.println("Gyro");
                imuSerial.write(CAL_GYR);
                break;
          }
        }else{
          return 1;
        }
      }
  return 0;
}
//store della calibrazione
int CMPS14::store(){
    for (int i = 0; i < 3; i++) {
         imuSerial.write(storing[i]);
        while ( imuSerial.available() < 1);
        byte ok_check =  imuSerial.read();
        if (ok_check == 0x55) {
          return 0;
        }
      }
}
/*
 * Combinazione di due byte, questo è dato dal fatto che la seriale legge un solo byte alla volta
 */
int16_t CMPS14::merge2Byte(){
  tmp1byte = imuSerial.read();
  tmp2byte = tmp1byte;
  tmp1byte = imuSerial.read();
  tmp2byte = ((tmp2byte << 8) | tmp1byte);
  return tmp2byte;
}
/*******************************
 *Conversione necessaria, al fine di avere un numero da Q-form a float, il Q-form è il modo in cui l'imu ci manda i dati
 *
 *
 *
 */
double CMPS14::QForm2Float(int16_t qData){
  return qData/pow(2,8);
}
// Prende lo stato di calibrazione
byte CMPS14::Get_status(){
    imuSerial.write(CMPS_GET_CALIBRATION_STATE); // Request current calibration
    while (imuSerial.available() < 1);
    byte calibration = imuSerial.read();
            // Display calibration
    //printBinary(calibration);
    return calibration;
}
double CMPS14::get_roll(){
    imuSerial.write(ROLL);
    // while(imuSerial.available() < 2);
    
    // return (double)CMPS14::merge2Byte()/10.0; 
    return 1;
}
double CMPS14::get_pitch(){
    imuSerial.write(PITCH);
    // while(imuSerial.available() < 2);
    
    // return (double)CMPS14::merge2Byte()/10.0;
    return 1;
}
void CMPS14::get_accXYZ(double* arrayACC){
    imuSerial.write(ACC);
    // while(imuSerial.available() < 6);
    // arrayACC[0] = CMPS14::QForm2Float(CMPS14::merge2Byte());//Effettuo una conversione da int_16_t a double, quell'intero è codificato in Q-form, vedi QForm2Float
    // arrayACC[1] = CMPS14::QForm2Float(CMPS14::merge2Byte());
    // arrayACC[2] = CMPS14::QForm2Float(CMPS14::merge2Byte());
    //return array
}
void CMPS14::get_LINaccXYZ(double* array_LIN_ACC){
  imuSerial.write(LIN_ACC);
  // while(imuSerial.available() < 6);
  // array_LIN_ACC[0] = CMPS14::QForm2Float(CMPS14::merge2Byte());
  // array_LIN_ACC[1] = CMPS14::QForm2Float(CMPS14::merge2Byte());
  // array_LIN_ACC[2] = CMPS14::QForm2Float(CMPS14::merge2Byte());
}
void SERCOM3_Handler()    // Interrupt handler for SERCOM3
{
  imuSerial.IrqHandler();
}

bool CMPS14::Read_all_data(){
  if(imuSerial.available() >= 16){
       
        CMPS14::accXYZ_value[0] = QForm2Float(merge2Byte());//Effettuo una conversione da int_16_t a double, quell'intero è codificato in Q-form, vedi QForm2Float
        CMPS14::accXYZ_value[1] = QForm2Float(merge2Byte());
        CMPS14::accXYZ_value[2] = QForm2Float(merge2Byte());
       
        CMPS14::pitch_value=(double)CMPS14::merge2Byte()/10.0;
        CMPS14::role_value=(double)CMPS14::merge2Byte()/10.0;
        
        CMPS14::acc_lin_value[0] = QForm2Float(merge2Byte());
        CMPS14::acc_lin_value[1] = QForm2Float(merge2Byte());
        CMPS14::acc_lin_value[2] = QForm2Float(merge2Byte());
   
        return true;
  }else{
    return false;
  }
}