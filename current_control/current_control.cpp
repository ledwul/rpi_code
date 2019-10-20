#include "current_control.h"

  const int alert_pin = 26;
ADS1115_ADC::ADS1115_ADC(int i2c_addr, float shunt_ohms): i2c_addr_(i2c_addr), shunt_ohms_(shunt_ohms){

  try{
    fd_ = open("/dev/i2c-1", O_RDWR);
    if (fd_ < 0) throw 2;
  }
  catch(int e){
    fprintf(stderr, "Failed to open i2c bus\n");
  }
  gpioSetMode(alert_pin, PI_INPUT);

  selectDevice(fd_, i2c_addr_, "ADS1115");
}

float ADS1115_ADC::getVoltage (int channel){
  long sum = 0;
  const int n_samples = 1;
  short output_buffer = prev_output;
  for (int i = 0; i < n_samples; i++){
    int cnt = 0;
    while((readShort(fd_, config_adr) & 0b1000000000000000) == 0) cnt++; // wait for previous conversion to finish
    output_buffer = readShort(fd_, result_adr);
    
    //                                    1222333455567899
    writeToDeviceShort(fd_, config_adr, 0b1100100111100011 | (channel << 12));

    
    
    
    
    return output_buffer/(32768.0*0.512);
    sum += output_buffer;
  }
  prev_output = output_buffer;
  return sum/32768.0/n_samples*0.512;
}

float ADS1115_ADC::getCurrent(int channel){
  float voltage = getVoltage(channel);
  return (voltage - offsets_.at(channel)) / shunt_ohms_;
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
    float sum = 0;
    offsets_[i] = 0;
    for (int j = 0; j < 100; j++)
      sum += getVoltage(i);
    offsets_[i] = sum / 100.0;
    std::cerr << "Offset for channel " << i << " is set to " << offsets_[i] << "V." << std::endl;
  }
}

