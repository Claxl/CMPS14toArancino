#include <SD.h> //Serve ad usare i file
#include <SoftwareSerial.h>// Comunica con l'imu

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

SoftwareSerial CMPS14(2, 3);

byte calibration_code[3] = {0x98, 0x95, 0x99}, ok_check; //byte necessari per attivare la ricalibrazione
byte storing [3] = {0xF0, 0xF5, 0xF6};//Byte necessari allo storage della calibrazione
byte calibration; //Byte per conservare la calibrazione
File file;
int8_t tmp1byte; //Temp byte al fine di effetturare il merge tra 2 byte
int16_t tmp2byte; //2 byte temporanei post merge
double roll,pitch,accX,accY,accZ, LINaccX, LINaccY, LINaccZ;

/*
 * Questa funzione ha lo scopo di visualizzare bit per bit un byte.
 * Si potrebbe rendere modulare inviando la grandezza della stringa di bit
 */
void printBinary(byte inByte){
  for (int b = 7; b >= 0; b--)
  {
    Serial.print(bitRead(inByte, b));
  }
  Serial.println("");
}


/*
 * Effettua automaticamente il merge di due byte, restituisce un intero, quindi nella chiamata può risultare necessario il casting
 */
int16_t merge2Byte(){
  tmp1byte = CMPS14.read();
  tmp2byte = tmp1byte;
  tmp1byte = CMPS14.read();
  tmp2byte = ((tmp2byte << 8) | tmp1byte);
  return tmp2byte;
}


/*
 * Converte il formato d'uscita dell'accelerometro in double, https://www.techtarget.com/whatis/definition/Q-format.
 * Per la conversione è stata usata la seguente formula, https://stackoverflow.com/questions/49678055/how-do-i-convert-q-format-integers-to-float-or-vice-versa.
 * Per renderla generale, passare il dato che ad oggi è 8, esso rappresenta la potenza del due con la quale è codificato il formato.
 */
double QForm2Float(int16_t qData){
  return qData/pow(2,8);
}

/*
 * Questa funzione permette di effettuare la calibrazione, essa va effettuata secondo le modalità descritte dal datasheet
 * Tilting di 45 e 90 gradi per accelerometro
 * Stazionario su di un piano per il giroscopio
 */
void cal(char type){
  Serial.println("----------------------");
  Serial.println("   Calibrate CMPS14   ");
  Serial.println("----------------------");
      //Calibration code
      for (int i = 0; i < 3; i++) {
        Serial.println("Start :");
        CMPS14.write(calibration_code[i]);
        ok_check = CMPS14.read();
        if (ok_check == 0x55) {
          Serial.print("\nCalibration Ok ");
          Serial.print(i, DEC);
        }
      }
      switch(type){
          case 'a':
            Serial.println("Accel");
            CMPS14.write(CAL_ACC);
            break;
          case 'g':
            Serial.println("Gyro");
            CMPS14.write(CAL_GYR);
            break;
      }
          
      Serial.print("\nCalibra adesso\n");
      delay(10000);
}


/*
 * Permette lo storage della calibrazione
 */
void store(){
  Serial.println("----------------------");
  Serial.println("   Store CMPS14       ");
  Serial.println("----------------------");
  for (int i = 0; i < 3; i++) {
        CMPS14.write(storing[i]);
        while (CMPS14.available() < 1);
        ok_check = CMPS14.read();
        if (ok_check == 0x55) {
          Serial.print("\nStore Ok ");
          Serial.print(i, DEC);
        }
      }
}

/*
 * Ci permette di invocare il PID della calibrazione salvata e quindi vedere lo stato di quest'ultima
 */
void get_cal(){
    CMPS14.write(CMPS_GET_CALIBRATION_STATE); // Request current calibration
    while (CMPS14.available() < 1);
    calibration = CMPS14.read();
    Serial.print("\nCalibration: ");            // Display calibration
    printBinary(calibration);
}


/*
 * Prendiamo il roll
 */
double get_roll(){
  CMPS14.write(ROLL);
  while(CMPS14.available() < 2);
  return (double)merge2Byte()/10.0; // casting da int16_t a double
}

/*
 * Prendiamo il pitch
 */
double get_pitch(){
  CMPS14.write(PITCH);
  while(CMPS14.available() < 2);
  return (double)merge2Byte()/10.0;// casting da int16_t a double
}

/*
 * Prendiamo tutte le accelerazioni
 */
void get_acc(){
  CMPS14.write(ACC);
  while(CMPS14.available() < 6);
  accX = QForm2Float(merge2Byte());//Effettuo una conversione da int_16_t a double, quell'intero è codificato in Q-form, vedi QForm2Float
  accY = QForm2Float(merge2Byte());
  accZ = QForm2Float(merge2Byte());
}

/*
 * Prendo tutte le accelerazioni lineari, non considero l'accelerazione gravitazionale
 */
void get_LINacc(){
  CMPS14.write(LIN_ACC);
  while(CMPS14.available() < 6);
  LINaccX = QForm2Float(merge2Byte());
  LINaccY = QForm2Float(merge2Byte());
  LINaccZ = QForm2Float(merge2Byte());
}



void setup() {
  Serial.begin(9600);
  CMPS14.begin(9600);
  /*
   * Avvio SD, per il log
   */
  if (!SD.begin(PIN_SPI_CS)) {
    Serial.println("SD CARD FAILED, OR NOT PRESENT!");
    while (1); // don't do anything more:
  }
  SD.remove("arduino.txt"); // delete the file if existed
  // create new file by opening file for writing
  file = SD.open("arduino.txt", FILE_WRITE);

  
  /*
   * Si potrebbe rendere facoltativo
   */
  cal('a');//Calibrazione accelerometro
  cal('g');//Calibrazione giroscopio
  store();//Salvataggio
  get_cal();//Visualizzazione calibrazione
}

void loop() {
  file = SD.open("arduino.txt", FILE_WRITE);
  file.seek(EOF);
  if (file) { //solo se c'è il file
    roll = get_roll();
    Serial.print("ROLL:\t\t\t");
    Serial.println(roll);
    file.print("ROLL:\t\t\t");
    file.print(roll);
    pitch = get_pitch();
    Serial.print("PITCH:\t\t\t");
    Serial.println(pitch);
    file.print("PITCH:\t\t\t");
    file.print(pitch);
    get_acc();
    Serial.println("Acceleration X Y Z");
    file.println("Acceleration X Y Z");
    Serial.print(accX);
    Serial.print("\t\t");
    Serial.print(accY);
    Serial.print("\t\t");
    Serial.print(accZ);
    file.print(accX);
    file.print(accY);
    file.print(accZ);
    get_LINacc();
    Serial.println("Linear acceleration X Y Z");
    file.println("Linear acceleration X Y Z");
    Serial.print(LINaccX);
    Serial.print("\t\t");
    Serial.print(LINaccY);
    Serial.print("\t\t");
    Serial.print(LINaccZ);
    file.print(LINaccX);
    file.print(LINaccY);
    file.print(LINaccZ);
    file.close();
  } else {
    Serial.print("SD Card: error on opening file arduino.txt");
  }
}
