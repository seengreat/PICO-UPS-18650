#ifndef __INA219_H
#define __INA219_H

/*-------------------------*/
/* INA219 Register Address */
/*-------------------------*/
/** 1.A0-SCL   A1-GND i2c slave address is 0X43 **/
#define SLAVE_ADDRESS   0X43

/** 2.All-register reset setting for bus voltage,PGA Gain ADC resolution/averaging **/
#define CONFIG_ADDR  0X00

/** 3.Shut voltage measurement data **/
#define SHUNT_VOLTAGE_ADDR  0X01

/** 4.Bus voltage measurement data **/
#define BUS_VOLTAGE_ADDR 0X02

/** 5.Bus voltage measurement data **/
#define POWER_ADDR   0X03

/** 6.Contains the value of the current flowing through the shunt resistor **/
#define CURRENT_ADDR 0X04

/** 7.Set full-scale range and LSB of current and power measurements.Overall system cailbration **/
#define CALIBRATION_ADDR 0X05

/*--------------------------------------*/
/* INA219 Configuration registers value */
/*--------------------------------------*/
/** 1.Bit 15: INA219 Configuration reset.Resets all regisiters to default **/
#define CONFIG_RESET     0X8000

/** 2.Bit 13: INA219 Bus voltage range **/
#define BUS_VOLTAGE_RANGE_16V    0x0000
#define BUS_VOLTAGE_RANGE_32V    0x2000

/** 3.Bit 11,12: INA219 PGA gain and range **/
#define PGA_GAIN_1_RANGE_40mv    0x0000
#define PGA_GAIN_2_RANGE_80mv    0x0800
#define PGA_GAIN_4_RANGE_160mv   0x1000
#define PGA_GAIN_5_RANGE_320mv   0x1800

/** 4.Bit 7-10: INA219 Bus ADC resolution **/
#define BADC_RESOLUTION_9Bit     0x0000  //Conversion time 84us      9-bit bus adc resolution    0..511
#define BADC_RESOLUTION_10Bit    0x0080  //Conversion time 148us     10-bit bus adc resolution   0..1023
#define BADC_RESOLUTION_11Bit    0x0100  //Conversion time 276us     11-bit bus adc resolution   0..2047
#define BADC_RESOLUTION_12Bit    0x0180  //Conversion time 532us     12-bit bus adc resolution   0..4095

/** 5.Bit 3-6: INA219 Shunt ADC resolution and the number of samples used when averaging results for the Shunt Voltage Register **/
#define SADC_RESOLUTION_9Bit_1S      0x0000  //Conversion time 84us      9-bit shunt adc resolution      1 sample
#define SADC_RESOLUTION_10Bit_1S     0x0008  //Conversion time 148us     10-bit shunt adc resolution     1 sample
#define SADC_RESOLUTION_11Bit_1S     0x0010  //Conversion time 276us     11-bit shunt adc resolution     1 sample
#define SADC_RESOLUTION_12Bit_1S     0x0018  //Conversion time 532us     12-bit shunt adc resolution     1 sample
#define SADC_RESOLUTION_12Bit_2S     0x0048  //Conversion time 1.06ms    12-bit shunt adc resolution     2 samples
#define SADC_RESOLUTION_12Bit_4S     0x0050  //Conversion time 2.13ms    12-bit shunt adc resolution     4 samples
#define SADC_RESOLUTION_12Bit_8S     0x0058  //Conversion time 4.26ms    12-bit shunt adc resolution     8 samples
#define SADC_RESOLUTION_12Bit_16S    0x0060  //Conversion time 8.51ms    12-bit shunt adc resolution     16 samples
#define SADC_RESOLUTION_12Bit_32S    0x0068  //Conversion time 17.02ms   12-bit shunt adc resolution     32 samples
#define SADC_RESOLUTION_12Bit_64S    0x0070  //Conversion time 34.05ms   12-bit shunt adc resolution     64 samples
#define SADC_RESOLUTION_12Bit_128S   0x0078  //Conversion time 68.10ms   12-bit shunt adc resolution     128 samples

/** 6.Bit 0-2: INA219 Operating Mode set **/
#define MODE_POWER_DOWN           0
#define MODE_SVOLT_TRIGGERED      1
#define MODE_BVOLT_TRIGGERED      2
#define MODE_SBVOLT_TRIGGERED     3
#define MODE_ADC_OFF              4
#define MODE_SVOLT_CONTINUOUS     5
#define MODE_BVOLT_CONTINUOUS     6
#define MODE_SBVOLT_CONTINUOUS    7

/*------------*/
/* LED Pinout */
/*------------*/
#define LED(x) gpio_put(PICO_DEFAULT_LED_PIN, x)

typedef struct
{
    /* data */
    int INA219_CalValue; //Ca
    float INA219_CurrentDivider_mA;
    float INA219_PowerMultiplier_mW;
    float Shunt_Voltage_mV;
    float Bus_Voltage_V;
    float Current_mA;
    float Power_mW;
}INA219Struct;

INA219Struct INA219={0,0.0,0.0,0.0,0.0,0.0,0.0};

#endif
