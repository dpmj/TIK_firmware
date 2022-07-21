//#include <freertos/FreeRTOS.h>
#include "ADCSampler.h"

ADCSampler::ADCSampler(adc_unit_t adcUnit, adc1_channel_t adcChannel, const i2s_config_t &i2s_config) {
    m_adcUnit = adcUnit;
    m_adcChannel = adcChannel;
    m_i2sPort = I2S_NUM_0;
    m_i2s_config = i2s_config;
}

void ADCSampler::start() {
    i2s_driver_install(m_i2sPort, &m_i2s_config, 0, NULL);  // install the i2s driver
    i2s_set_adc_mode(m_adcUnit, m_adcChannel);              // Init ADC
    i2s_adc_enable(m_i2sPort);                              // Enable ADC
}

void ADCSampler::stop() {
    i2s_adc_disable(m_i2sPort);                             // avoid locking the ADC
    i2s_driver_uninstall(m_i2sPort);                        // stop the i2S driver
}

uint16_t ADCSampler::read(int16_t *samples, uint16_t count) {
    size_t bytes_read = 0;  // the nº of bytes read may not be same as 'count'

    i2s_read(m_i2sPort, (void*) samples, sizeof(uint16_t) * count, &bytes_read, portMAX_DELAY);

    for (int i = 0; i < count; i++) {
        samples[i] = samples[i] & 0x0FFF; // (2048 - (uint16_t(samples[i]) & 0xfff)) * 15;
    }

    return (uint16_t) bytes_read / sizeof(int16_t);
}
