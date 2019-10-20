#include "current_control.h"

ADS1115_ADC::ADS1115_ADC(int i2c_addr, int config, int result, int shunt_ohms): i2c_addr_(i2c_addr), config_(config), result_(result), shunt_ohms_(shunt_ohms){

        try{
            fd_ = open("/dev/i2c-1", O_RDWR);
            if (fd_ < 0) throw 2;
        }
        catch(int e){
            fprintf(stderr, "Failed to open i2c bus\n");
        }

        selectDevice(fd_, i2c_addr_, "ADS1115");
    }

float ADS1115_ADC::getVoltage (int channel){
    long sum = 0;
    const int n_samples = 1;
    short output_buffer;
    for (int i = 0; i < n_samples; i++){
        writeToDeviceShort(fd_, config_, 0b1100100111100011 | (channel << 12));
        usleep(1200);
        output_buffer = readShort(fd_, result_);
        sum += output_buffer;
    }
    return sum/32768.0/n_samples*0.512;
}

float ADS1115_ADC::getCurrent(int channel){
    return (getVoltage(channel) - offsets_.at(channel)) / shunt_ohms_;
}

void ADS1115_ADC::setOffsets(ports_t ports){
    //Check of offset is already populated, populate if not
    if(offsets_.size() == 0){
        offsets_.resize(ports.size());
    }
    //else check if sizes agree
    else{
        assert(offsets_.size() == ports.size());
    }

    for(int i = 0; i < 4; i++){
        //Set pwm output to zero
        gpioWrite(ports.at(i), 0);
        usleep(100000);
        //Get the voltage on zero pwm
        offsets_.at(i) = getVoltage(i);
    }
}

