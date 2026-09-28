#ifndef CC2500_H
#define CC2500_H

#include <driver/spi_master.h>
#include <driver/gpio.h>

#define LOGIC_LOW 0
#define LOGIC_HIGH 1

#define CC2500_READ_FLAG 0x80
#define CC2500_WRITE_FLAG 0x00
#define CC2500_BURST_FLAG 0x40


typedef struct {
    spi_host_device_t host;
    gpio_num_t sclk_pin;
    gpio_num_t mosi_pin;
    gpio_num_t miso_pin;
    bool initialised;
    size_t device_count;
} cc2500_bus_t;


typedef struct {
    spi_device_handle_t handle;
    cc2500_bus_t *bus;
    int clock_speed_hz;
    gpio_num_t cs_pin;
    gpio_num_t gdo0_pin;
    gpio_num_t gdo2_pin;
} cc2500_t;



#endif // CC2500_H
