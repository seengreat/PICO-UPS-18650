from machine import I2C,Pin
import time

# INA219 Register Address
# 1.A0-SCL   A1-GND i2c slave address is 0X43
SLAVE_ADDRESS = 0X43
# 2.All-register reset setting for bus voltage,PGA Gain ADC resolution/averaging
CONFIG_ADDR = 0X00
# 3.Shut voltage measurement data
SHUNT_VOLTAGE_ADDR = 0X01
# 4.Bus voltage measurement data
BUS_VOLTAGE_ADDR = 0X02
# 5.Bus voltage measurement data
POWER_ADDR = 0X03
# 6.Contains the value of the current flowing through the shunt resistor
CURRENT_ADDR = 0X04
# 7.Set full-scale range and LSB of current and power measurements.Overall system cailbration
CALIBRATION_ADDR = 0X05

# INA219 Configuration registers value
# 1.Bit 15: INA219 Configuration reset.Resets all regisiters to default
RESET = 0X8000

# 2.Bit 13: INA219 Bus voltage range
BUS_VOLTAGE_RANGE_16V = 0x0000
BUS_VOLTAGE_RANGE_32V = 0x2000

# 3.Bit 11,12: INA219 PGA gain and range
PGA_GAIN_1_RANGE_40mv = 0x0000
PGA_GAIN_2_RANGE_80mv = 0x0800
PGA_GAIN_4_RANGE_160mv = 0x1000
PGA_GAIN_5_RANGE_320mv = 0x1800

# 4.Bit 7-10: INA219 Bus ADC resolution
BADC_RESOLUTION_9Bit = 0x0000  # Conversion time 84us
BADC_RESOLUTION_10Bit = 0x0080  # Conversion time 148us
BADC_RESOLUTION_11Bit = 0x0100  # Conversion time 276us
BADC_RESOLUTION_12Bit = 0x0180  # Conversion time 532us

# 5.Bit 3-6: INA219 Shunt ADC resolution and the number of samples used when averaging results for the Shunt Voltage Register
SADC_RESOLUTION_9Bit_1S = 0x0000      #Conversion time 84us      9-bit shunt adc resolution      1 sample
SADC_RESOLUTION_10Bit_1S = 0x0008     #Conversion time 148us     10-bit shunt adc resolution     1 sample
SADC_RESOLUTION_11Bit_1S = 0x0010     #Conversion time 276us     11-bit shunt adc resolution     1 sample
SADC_RESOLUTION_12Bit_1S = 0x0018     #Conversion time 532us     12-bit shunt adc resolution     1 sample
SADC_RESOLUTION_12Bit_2S = 0x0048     #Conversion time 1.06ms    12-bit shunt adc resolution     2 samples
SADC_RESOLUTION_12Bit_4S = 0x0050     #Conversion time 2.13ms    12-bit shunt adc resolution     4 samples
SADC_RESOLUTION_12Bit_8S = 0x0058     #Conversion time 4.26ms    12-bit shunt adc resolution     8 samples
SADC_RESOLUTION_12Bit_16S = 0x0060    #Conversion time 8.51ms    12-bit shunt adc resolution     16 samples
SADC_RESOLUTION_12Bit_32S = 0x0068    #Conversion time 17.02ms   12-bit shunt adc resolution     32 samples
SADC_RESOLUTION_12Bit_64S = 0x0070    #Conversion time 34.05ms   12-bit shunt adc resolution     64 samples
SADC_RESOLUTION_12Bit_128S = 0x0078   #Conversion time 68.10ms   12-bit shunt adc resolution     128 samples

# 6.Bit 0-2: INA219 Operating Mode set
MODE_POWER_DOWN = 0
MODE_SVOLT_TRIGGERED = 1
MODE_BVOLT_TRIGGERED = 2
MODE_SBVOLT_TRIGGERED = 3
MODE_ADC_OFF = 4
MODE_SVOLT_CONTINUOUS = 5
MODE_BVOLT_CONTINUOUS = 6
MODE_SBVOLT_CONTINUOUS = 7


class INA219():
    def __init__(self):
        self.i2c = I2C(1, scl=Pin(3), sda=Pin(2), freq=400000)
        self.i2c.scan()
        self.addr = SLAVE_ADDRESS
         
        self.config = BUS_VOLTAGE_RANGE_32V |\
            PGA_GAIN_5_RANGE_320mv |\
            BADC_RESOLUTION_12Bit |\
            SADC_RESOLUTION_12Bit_32S |\
            MODE_SBVOLT_CONTINUOUS

        # !
        # According to the equation of data sheet,can get the value of calibration.
        #              Cal = trunc(0.04096/(Current_LSB*RSHUNT))
        #  where
        #       > 0.04096 is an internal fixed value used to ensure scaling is maintained properly
        #       > Current_LSB = Maximum Expected Current/2^15
        #
        #  1. We should calculate the value of Maximum expected current:
        #     a. According to the value of config:
        #                                         > VBUS_MAX = 32 V
        #                                         > VSHUNT_MAX = 0.32 V
        #                                         > RSHUNT = 0.01 Ohm (Determined by hardware)
        #     b. Maximum Expected Current = VSHUNT_MAX/RSHUNT
        #                                 = 0.32/0.01
        #                                 = 32A
        #  2. Current_LSB = Maximum Expected Current/2^15
        #                 = 32/2^15
        #                 ≈ 0.001A (1mA)
        #  3. Cal = trunc(0.04096/(Current_LSB*RSHUNT))
        #         = trunc(0.04096/(0.001*0.01))
        #         = 4096
        #
        self.INA219_CalValue = 4096
        # Set Calibration register to 'Cal' calculated above
        self.INA219_WriteReg(CALIBRATION_ADDR, self.INA219_CalValue)

        # !
        # According to the equation of data sheet,Power_LSB = 20*Current_LSB
        #                                                   = 20*0.001
        #                                                   = 0.02 W
        #
        self.INA219_PowerMultiplier_mW = 20.0  # Power_LSB 20mW
        self.INA219_CurrentDivider_mA = 1.0  # Current_LSB 1mA

        self.INA219_WriteReg(CONFIG_ADDR, self.config)

        self.Shunt_Voltage_mV = 0.0
        self.Bus_Voltage_V = 0.0
        self.Current_mA = 0.0
        self.Power_mW = 0.0

    # @brief  Write a 16-bit data to INA219
    # @param  Register address
    #         16-bit data
    # @return None
    def INA219_WriteReg(self, addr, data):
        temp = [0, 0]
        temp[0] = (data >> 8) & 0XFF
        temp[1] = data & 0XFF
        self.i2c.writeto_mem(self.addr, addr, bytes(temp))

    # @brief  Read a 16-bit data from INA219
    # @param  Register address
    # @return Register data
    def INA219_Read(self, addr):
        temp = self.i2c.readfrom_mem(self.addr, addr, 2)
        data = temp[0] << 8 | temp[1]
        return data

    # @brief  Get the value of shunt voltage in mV
    # @param  None
    # @return The value of shunt voltage in mV
    def INA219_Read_SVOLT_mV(self):
        INA219_Shunt_Voltage = self.INA219_Read(SHUNT_VOLTAGE_ADDR)
        if INA219_Shunt_Voltage > 32767:
            INA219_Shunt_Voltage -= 65535
        return INA219_Shunt_Voltage * 0.01

    # @brief  Get the value of bus voltage in V
    # @param  None
    # @return The value of bus voltage in V
    def INA219_Read_BVOLT_V(self):
        INA219_Bus_Voltage = self.INA219_Read(BUS_VOLTAGE_ADDR)
        return (INA219_Bus_Voltage >> 3) * 0.004

    # @brief  Get the value of current in mA
    # @param  None
    # @return The value of current in mA
    def INA219_Read_Current_mA(self):
        # Because current full-scale range and LSB depend on the value enterd in
        # the Calibration register,we reset the Cal register again to read the
        # current register safetly
        self.INA219_WriteReg(CALIBRATION_ADDR, self.INA219_CalValue)
        time.sleep(0.02)
        INA219_Current = self.INA219_Read(CURRENT_ADDR)
        
        if INA219_Current > 32767:
            INA219_Current -= 65535
            
        return INA219_Current * self.INA219_CurrentDivider_mA

    # @brief  Get the value of power in mW
    # @param  None
    # @return The value of power in mW
    def INA219_Read_Power_mW(self):
        # Because power full-scale range and LSB depend on the value enterd in
        # the Calibration register,we reset the Cal register again to read the
        # current register safetly.
        self.INA219_WriteReg(CALIBRATION_ADDR, self.INA219_CalValue)
        time.sleep(0.02)
        INA219_Power = self.INA219_Read(POWER_ADDR)
        return INA219_Power * self.INA219_PowerMultiplier_mW


if __name__=='__main__':
    ina219 = INA219()
    LED = machine.Pin(25,machine.Pin.OUT)
    while True:
        #Shunt_voltage = ina219.INA219_Read_SVOLT_mV()
        Bus_voltage = ina219.INA219_Read_BVOLT_V()
        Current = ina219.INA219_Read_Current_mA()
        Power = ina219.INA219_Read_Power_mW()
        P = (Bus_voltage - 3.0) / 1.2 * 100;
        if (P<0):
            P = 0
        elif (P>100):
            p = 100
        #print("Shunt voltage: {:6.1f} mV".format(Shunt_voltage))
        print("Voltage: {:6.1f} V".format(Bus_voltage))
        print("Current: {:6.3f} A".format(Current/1000.0))
        print("Power:   {:6.3f} W".format(Power/1000.0))
        print("Percent: {:6.0f}%".format(P))
        print("\n")
        LED.toggle()
        time.sleep(1)


