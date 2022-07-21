/**
 * @brief A i2S and DMA sampler designed to achieve high sampling frequencies from the
 * ESP32's internal ADC.
 *  
 * Forked from the work of Chris Greening: https://github.com/atomic14/esp32_audio
 *
 * --- Memory usage:
 * 
 * Samples are sent directly to RAM buffers from the ADC controller using i2S. The amount
 * of buffers reserved to this is set by dma_buf_count and the size of every buffer by 
 * .dma_buf_len (= 1024 max.)
 * So, if we have 4 buffers at 1024 Samples/buffer and with a sample size of 16 bits:
 * RAM_USAGE = 4 buffers · 1024 Sa/buffer · 16 bpSa / 8 bpByte = 8192 Bytes = 8 KB
 * 
 * --- IMPORTANT NOTE: i2S sends data in pairs as if it were two channels, even if the 
 * communication format is I2S_CHANNEL_FMT_ONLY_LEFT, so samples MUST BE REORDERED as seen
 * on these forum posts:
 * 
 * https://www.esp32.com/viewtopic.php?t=5522
 * https://www.esp32.com/viewtopic.php?t=11023
 * 
 * When I2S_CHANNEL_FMT_ALL_LEFT is set, samples are duplicated but are received at double
 * the speed of Fs. This method produces good results and samples don't need to be 
 * reordered but it uses double the memory to capture the same time interval. 
 * 
 */


#pragma once

#include <driver/i2s.h>

class ADCSampler {
private:
    adc_unit_t m_adcUnit;
    adc1_channel_t m_adcChannel;

protected:
    i2s_port_t m_i2sPort = I2S_NUM_0;
    i2s_config_t m_i2s_config;
    void configureI2S();
    void unConfigureI2S();

public:
    ADCSampler(adc_unit_t adc_unit, adc1_channel_t adc_channel, const i2s_config_t &i2s_config);
    uint16_t read(int16_t *samples, uint16_t count);
    void stop();
    void start();
};
