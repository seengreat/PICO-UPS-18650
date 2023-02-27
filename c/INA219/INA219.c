#include "INA219.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
/*!
 *  @brief  LED initialization
 *  @param  None
 *  @return None
 */
void LED_Init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

/*!
 *  @brief  INA219_I2C initialization
 *  @param  None
 *  @return None
 */
void INA219_i2cInit()
{
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}

/*!
 *  @brief  Write a 16-bit data to INA219
 *  @param  Register address
 *          16-bit data
 *  @return None
 */
void INA219_WriteReg(uint8_t addr, uint16_t data)
{
    uint8_t temp[3];
    temp[0] = addr;
    temp[1] = (data >> 8) & 0XFF;
    temp[2] = data & 0XFF;

    i2c_write_blocking(i2c1, SLAVE_ADDRESS, temp, 3, false);
}

/*!
 *  @brief  Read a 16-bit data from INA219
 *  @param  Register address
 *          Data address
 *  @return None
 */
void INA219_ReadReg(uint8_t addr, int16_t *data)
{
    uint8_t temp[2];
    i2c_write_blocking(i2c1, SLAVE_ADDRESS, &addr, 1, true);
    i2c_read_blocking(i2c1, SLAVE_ADDRESS, temp, 2, false);
    *data = (((int16_t)temp[0] << 8) | (int16_t)temp[1]);
}

/*!
 *  @brief  Set Configuration register and Calibration register
 *  @param  None
 *  @return None
 */
void INA219_Config_And_CalibrationSet()
{
    uint16_t config = BUS_VOLTAGE_RANGE_32V | PGA_GAIN_5_RANGE_320mv |
                      BADC_RESOLUTION_12Bit | SADC_RESOLUTION_12Bit_32S |
                      MODE_SBVOLT_CONTINUOUS;
    /*!
     *  According to the equation of data sheet,can get the value of calibration.
     *               Cal = trunc(0.04096/(Current_LSB*RSHUNT))
     *   where
     *        > 0.04096 is an internal fixed value used to ensure scaling is maintained properly
     *        > Current_LSB = Maximum Expected Current/2^15
     *
     *   1. We should calculate the value of Maximum expected current:
     *      a. According to the value of config:
     *                                          > VBUS_MAX = 32 V
     *                                          > VSHUNT_MAX = 0.32 V
     *                                          > RSHUNT = 0.01 Ohm (Determined by hardware)
     *      b. Maximum Expected Current = VSHUNT_MAX/RSHUNT
     *                                  = 0.32/0.01
     *                                  = 32A
     *   2. Current_LSB = Maximum Expected Current/2^15
     *                  = 32/2^15
     *                  �� 0.001A (1mA)
     *   3. Cal = trunc(0.04096/(Current_LSB*RSHUNT))
     *          = trunc(0.04096/(0.001*0.01))
     *          = 4096
     */
    INA219.INA219_CalValue = 4096;
    // Set Calibration register to 'Cal' calculated above
    INA219_WriteReg(CALIBRATION_ADDR, INA219.INA219_CalValue);

    /*!
     *  According to the equation of data sheet,Power_LSB = 20*Current_LSB
     *                                                    = 20*0.001
     *                                                    = 0.02 W
     */
    INA219.INA219_PowerMultiplier_mW = 20.0; // Power_LSB 20mW
    INA219.INA219_CurrentDivider_mA = 1.0;   // Current_LSB 1mA

    // Set Config register to take into account the settings above
    INA219_WriteReg(CONFIG_ADDR, config);
}

/*!
 *  @brief  INA219 initialization
 *  @param  None
 *  @return None
 */
void INA219_Init()
{
    INA219_i2cInit();
    INA219_Config_And_CalibrationSet();
}

/*!
 *  @brief  Get the value of shunt voltage in mV
 *  @param  None
 *  @return The value of shunt voltage in mV
 */
float INA219_Read_SVOLT_mV()
{
    int16_t INA219_Shunt_Voltage = 0;
    INA219_ReadReg(SHUNT_VOLTAGE_ADDR, &INA219_Shunt_Voltage);
    return INA219_Shunt_Voltage * 0.01;
}

/*!
 *  @brief  Get the value of bus voltage in V
 *  @param  None
 *  @return The value of bus voltage in V
 */
float INA219_Read_BVOLT_V()
{
    int16_t INA219_Bus_Voltage = 0;
    INA219_ReadReg(BUS_VOLTAGE_ADDR, &INA219_Bus_Voltage);
    return (INA219_Bus_Voltage >> 3) * 4 * 0.001;
}

/*!
 *  @brief  Get the value of current in mA
 *  @param  None
 *  @return The value of current in mA
 */
float INA219_Read_Current_mA()
{
    int16_t INA219_Current = 0;
    /*!
     * Because current full-scale range and LSB depend on the value enterd in
     * the Calibration register,we reset the Cal register again to read the
     * current register safetly.
     */
    INA219_WriteReg(CALIBRATION_ADDR, INA219.INA219_CalValue);

    INA219_ReadReg(CURRENT_ADDR, &INA219_Current);

    if (INA219_Current < 0) // The charging current is positive and the discharging current is negative
        INA219_Current = -INA219_Current;

    return INA219_Current * INA219.INA219_CurrentDivider_mA;
}

/*!
 *  @brief  Get the value of power in mW
 *  @param  None
 *  @return The value of power in mW
 */
float INA219_Read_Power_mW()
{
    int16_t INA219_Power = 0;
    /*!
     * Because power full-scale range and LSB depend on the value enterd in
     * the Calibration register,we reset the Cal register again to read the
     * current register safetly.
     */
    INA219_WriteReg(CALIBRATION_ADDR, INA219.INA219_CalValue);

    INA219_ReadReg(POWER_ADDR, &INA219_Power);
    return INA219_Power * INA219.INA219_PowerMultiplier_mW;
}

int main()
{
    int LED_leval = 0;
    float P = 0;
    stdio_init_all();
    LED_Init();
    INA219_Init();
    while (1)
    {
        INA219.Shunt_Voltage_mV = INA219_Read_SVOLT_mV();
        INA219.Bus_Voltage_V = INA219_Read_BVOLT_V();
        INA219.Current_mA = INA219_Read_Current_mA();
        INA219.Power_mW = INA219_Read_Power_mW();
        P = (INA219.Bus_Voltage_V - 3) / 1.2 * 100;
        if (P < 0)
            P = 0;
        else if (P > 100)
            P = 100;
        //printf("Shunt voltage:  %6.1f mV\r\n", INA219.Shunt_Voltage_mV);
        printf("Voltage:        %6.1f V\r\n", INA219.Bus_Voltage_V);
        printf("Current:        %6.3f A\r\n", INA219.Current_mA / 1000.0);
        printf("Power:          %6.3f W\r\n", INA219.Power_mW / 1000.0);
        printf("Percent:        %6.1f %%\r\n", P);
        printf("\r\n");
        LED_leval = !LED_leval;
        LED(LED_leval);
        sleep_ms(1000);
    }
    return 0;
}
