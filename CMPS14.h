


#ifndef CMPS14_h
#define CMPS14_h
#endif

#include <Arduino.h>


class CMPS14{

    public:
        
        CMPS14();
        void BeginSerial(int baud);
        void setSerial(int TX, int RX);
         bool Read_all_data();
        int store();
        byte Get_status();
        double get_roll();
        double get_pitch();
        void get_accXYZ(double* arrayACC);
        void get_LINaccXYZ(double* array_LIN_ACC);
        int Calibration(char type);
        double pitch_value;
        double role_value;
        double accXYZ_value[3]={77,77,77};
        double acc_lin_value[3]={77,77,77};
    private:
        
        int16_t merge2Byte();
        
        double QForm2Float(int16_t qData);       
};