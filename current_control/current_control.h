#include <i2c.h>
#include <pigpio.h>
#include <cstdarg>
#include <vector>
#include <cassert>

using ports_t = std::vector<int>;
using voltages_t = std::vector<float>;

class ADS1115_ADC{
    private:
        const int i2c_addr_;
        const int config_;
        const int result_;
        const int shunt_ohms_;
        int fd_;
    public:
        voltages_t offsets_;
        /**
         * @brief Open selected i2c device.
         *
         * @param i2c_addr I2C adress of the selected device
         * @param config ?
         * @param result ?
         * @param shunt_ohms Resistance of the used shunt resistor
         */
        ADS1115_ADC(int i2c_addr, int config, int result, int shunt_ohms);
        /**
         * @brief Get the voltage measured on the selected channel
         *
         * @param channel Integer between 0 and 3 denoting one of the 4 channels of the ADS1115
         *
         * @return Voltage of the selected channel in [volt]
         */
        float getVoltage(int channel);
        /**
         * @brief Get the current on the selected channel
         *
         * @param channel Integer between 0 and 3 denoting of the 4 channels of the ADS1115
         *
         * @return Current of the selected channel in [ampere]
         */
        float getCurrent(int channel);

        /**
         * @brief Registers the voltage offsets on all channels by setting pwm output to zero and measuring the voltage over the shunt resistors.
         *
         * @param ports Vector of ints containing the pwm ports
         */
        void setOffsets(ports_t ports);
};
