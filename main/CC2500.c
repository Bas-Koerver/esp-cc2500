#include "cc2500.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#include "cc2500_reg.h"
#include "cc2500_strobe.h"

#define TAG "CC2500"
esp_err_t spi_transfer(cc2500_t* dev, spi_transaction_t* transaction) {
  esp_err_t ret;
  ret = gpio_set_level(dev->cs_pin, LOGIC_LOW);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = cc2500_wait_chip_rdyn(dev, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = spi_device_transmit(dev->handle, transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = gpio_set_level(dev->cs_pin, LOGIC_HIGH);
  if (ret != ESP_OK) {
    return ret;
  }

  return ESP_OK;
}

esp_err_t cc2500_wait_chip_rdyn(cc2500_t* dev, uint32_t timeout_us) {
  if (timeout_us == 0) {
    timeout_us = 150;  // Default timeout
  }

  if (dev == NULL || dev->bus == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  int start = esp_timer_get_time();
  while (esp_timer_get_time() - start <= timeout_us) {
    if (gpio_get_level(dev->bus->miso_pin) == 0) {
      return ESP_OK;
    }
    esp_rom_delay_us(1);
  }

  // Final check for edge cases where the if in the while loop was checked at
  // 149 μs, but the while loop restarted its iteration at 151 μs
  if (gpio_get_level(dev->bus->miso_pin) == 0) {
    return ESP_OK;
  }

  return ESP_ERR_TIMEOUT;
}

esp_err_t cc2500_read_reg(cc2500_t* dev, uint8_t reg, uint8_t* status,
                          uint8_t* value) {
  // 0x00 >= reg >= 0x2F: Only registers nothing below or above, like status
  // registers.
  if (dev == NULL || dev->handle == NULL || value == NULL || reg > 0x2F) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for single read/write the setup data needs to stay low
  // for at least 55 ns. With a clock speed larger than 9 MHz, the time would be
  // less than 55 ns, which could cause errors.
  if (dev->clock_speed_hz > 9000000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[2] = {reg | CC2500_READ_FLAG, 0x00};
  uint8_t rx[2] = {0};

  spi_transaction_t transaction = {
      .length = 16,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status) {
    *status = rx[0];
  }

  *value = rx[1];

  return ESP_OK;
}

esp_err_t cc2500_read_regs(cc2500_t* dev, const uint8_t* regs, size_t len,
                           uint8_t* status, uint8_t* values) {
  if (dev == NULL || dev->handle == NULL || regs == NULL || len == 0 ||
      values == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for burst read/write the setup data needs to stay low
  // for at least 76 ns. With a clock speed larger than 6,5 MHz, the time would
  // be less than 76 ns, which could cause errors.
  if (dev->clock_speed_hz > 6500000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t lowest_reg = 0xFF;
  uint8_t highest_reg = 0x00;
  // Find the lowest and highest register addresses.
  for (size_t i = 0; i < len; i++) {
    if (regs[i] < lowest_reg) {
      lowest_reg = regs[i];
    }
    if (regs[i] > highest_reg) {
      highest_reg = regs[i];
    }
  }

  size_t span = (size_t)(highest_reg - lowest_reg + 1U);

  // 0x00 >= reg >= 0x2F: Only registers nothing below or above, like status
  // registers.
  if (highest_reg > 0x2F) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[span + 1U];
  uint8_t rx[span + 1U];

  memset(tx, 0, sizeof(tx));
  memset(rx, 0, sizeof(rx));

  tx[0] = lowest_reg | CC2500_READ_FLAG | CC2500_BURST_FLAG;

  spi_transaction_t transaction = {
      .length = (span + 1U) * 8U,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status != NULL) {
    *status = rx[0];
  }

  for (size_t i = 0; i < len; i++) {
    values[i] = rx[(regs[i] - lowest_reg) + 1U];
  }

  return ESP_OK;
}

esp_err_t cc2500_write_reg(cc2500_t* dev, uint8_t reg, uint8_t* status,
                           uint8_t value) {
  // 0x00 <= reg <= 0x2F: Only registers nothing below or above, like status
  // registers.
  if (dev == NULL || dev->handle == NULL || reg > 0x2F) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for single read/write the setup data needs to stay low
  // for at least 55 ns. With a clock speed larger than 9 MHz, the time would be
  // less than 55 ns, which could cause errors.
  if (dev->clock_speed_hz > 9000000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[2] = {reg | CC2500_WRITE_FLAG, value};
  uint8_t rx[2] = {0};

  spi_transaction_t transaction = {
      .length = 16,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status) {
    *status = rx[1];
  }

  return ESP_OK;
}

esp_err_t cc2500_write_regs(cc2500_t* dev, const uint8_t reg, size_t len,
                            uint8_t* status, uint8_t* values) {
  // 0x00 <= reg <= 0x2F: Only registers nothing below or above, like status
  // registers.
  if (dev == NULL || dev->handle == NULL || len == 0 || values == NULL ||
      reg + len > 0x2F) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for burst read/write the setup data needs to stay low
  // for at least 76 ns. With a clock speed larger than 6,5 MHz, the time would
  // be less than 76 ns, which could cause errors.
  if (dev->clock_speed_hz > 6500000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[len + 1U];
  uint8_t rx[len + 1U];

  memset(tx, 0, sizeof(tx));
  memset(rx, 0, sizeof(rx));

  tx[0] = reg | CC2500_WRITE_FLAG | CC2500_BURST_FLAG;

  for (size_t i = 0; i < len; i++) {
    tx[i + 1U] = values[i];
  }

  spi_transaction_t transaction = {
      .length = (len + 1U) * 8U,  // ESP-IDF expects bits
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status != NULL) {
    *status = rx[1];
  }

  return ESP_OK;
}

esp_err_t cc2500_read_status_reg(cc2500_t* dev, uint8_t s_reg, uint8_t* status,
                                 uint8_t* value) {
  // 0x30 >= reg >= 0x3D: Only registers nothing below or above, like status
  // registers.
  if (dev == NULL || dev->handle == NULL || value == NULL || s_reg < 0x30 ||
      s_reg > 0x3D) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for single read/write the setup data needs to stay low
  // for at least 55 ns. With a clock speed larger than 9 MHz, the time would be
  // less than 55 ns, which could cause errors.
  if (dev->clock_speed_hz > 9000000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[2] = {s_reg | CC2500_READ_FLAG | CC2500_BURST_FLAG, 0x00};
  uint8_t rx[2] = {0};

  spi_transaction_t transaction = {
      .length = 16,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status) {
    *status = rx[0];
  }

  *value = rx[1];

  return ESP_OK;
}

esp_err_t cc2500_send_strobe(cc2500_t* dev, uint8_t strobe, uint8_t* status) {
  // 0x30 <= reg <= 0x3D: Only registers nothing below or above, like status
  // registers.
  if (dev == NULL || dev->handle == NULL || strobe < 0x30 || strobe > 0x3D) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for single read/write the setup data needs to stay low
  // for at least 55 ns. With a clock speed larger than 9 MHz, the time would be
  // less than 55 ns, which could cause errors.
  if (dev->clock_speed_hz > 9000000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[1] = {strobe | CC2500_WRITE_FLAG};
  uint8_t rx[1] = {0};

  spi_transaction_t transaction = {
      .length = 8,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status) {
    *status = rx[0];
  }

  return ESP_OK;
}

esp_err_t cc2500_read_fifo(cc2500_t* dev, uint8_t* status, uint8_t* data,
                           size_t len) {
  if (dev == NULL || dev->handle == NULL || data == NULL || len == 0) {
    return ESP_ERR_INVALID_ARG;
  }
  // Datasheet table 16 for burst read/write the setup data needs to stay low
  // for at least 76 ns. With a clock speed larger than 6,5 MHz, the time would
  // be less than 76 ns, which could cause errors.
  if (dev->clock_speed_hz > 6500000) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t tx[len + 1U];
  uint8_t rx[len + 1U];

  memset(tx, 0, sizeof(tx));
  memset(rx, 0, sizeof(rx));

  tx[0] = CC2500_FIFO_ADDR | CC2500_READ_FLAG | CC2500_BURST_FLAG;

  spi_transaction_t transaction = {
      .length = (len + 1U) * 8U,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };

  esp_err_t ret = spi_transfer(dev, &transaction);
  if (ret != ESP_OK) {
    return ret;
  }

  if (status != NULL) {
    *status = rx[0];
  }

  for (size_t i = 0; i < len; i++) {
    data[i] = rx[i + 1U];
  }

  return ESP_OK;
}

esp_err_t cc2500_reset(cc2500_t* dev) {
  // Datasheet chapter 19.1.2 Manual Reset.
  if (dev == NULL || dev->handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret = gpio_set_level(dev->bus->sclk_pin, LOGIC_LOW);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = gpio_set_level(dev->bus->mosi_pin, LOGIC_LOW);
  if (ret != ESP_OK) {
    return ret;
  }

  // Hold CSn high for at least 40 μs relative to pulling CSn low
  int start = esp_timer_get_time();

  ret = gpio_set_level(dev->cs_pin, LOGIC_LOW);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = gpio_set_level(dev->cs_pin, LOGIC_HIGH);
  if (ret != ESP_OK) {
    return ret;
  }

  while (esp_timer_get_time() - start < 40) {
    esp_rom_delay_us(1);
  }

  ret = gpio_set_level(dev->cs_pin, LOGIC_LOW);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = cc2500_wait_chip_rdyn(dev, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = cc2500_send_strobe(dev, CC2500_STROBE_SRES, NULL);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = cc2500_wait_chip_rdyn(dev, 0);
  if (ret != ESP_OK) {
    return ret;
  }

  return ESP_OK;
}

esp_err_t cc2500_bus_init(cc2500_bus_t* bus) {
  if (bus == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret;

  spi_bus_config_t buscfg = {.sclk_io_num = bus->sclk_pin,
                             .mosi_io_num = bus->mosi_pin,
                             .miso_io_num = bus->miso_pin,
                             // Not using WP and HD or Quad SPI, set to -1.
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1};

  ret = spi_bus_initialize(bus->host, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    return ret;
  }

  ESP_LOGI(
      TAG,
      "SPI bus initialized on pins clk: %d, mosi: %d, miso: %d. On host %d",
      bus->sclk_pin, bus->mosi_pin, bus->miso_pin, bus->host);
  return ESP_OK;
}

esp_err_t cc2500_init(cc2500_t* dev) {
  if (dev == NULL || dev->bus == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  dev->handle = NULL;
  esp_err_t ret;

  ret = gpio_reset_pin(dev->cs_pin);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = gpio_set_direction(dev->cs_pin, GPIO_MODE_OUTPUT);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = gpio_set_level(dev->cs_pin, LOGIC_HIGH);
  if (ret != ESP_OK) {
    return ret;
  }

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = dev->clock_speed_hz,
      .queue_size = 1,
      .mode = 0,
      // Manual chip select control -> datasheet chapter 10
      .spics_io_num = -1,
      .flags = SPI_DEVICE_NO_DUMMY};

  ret = spi_bus_add_device(dev->bus->host, &devcfg, &dev->handle);
  if (ret != ESP_OK) {
    return ret;
  }

  uint8_t partNumber;
  cc2500_read_status_reg(dev, CC2500_STATUS_PARTNUM, NULL, &partNumber);
  if (ret != ESP_OK) {
    return ret;
  }
  if (partNumber != 0x80) {
    ESP_LOGE(TAG,
             "Did not get correct Partnum, expected 0x80, got: 0x%02X.\nIs the "
             "chip properly connected?",
             partNumber);
    return ESP_ERR_INVALID_VERSION;
  }

  ESP_LOGI(TAG, "Attached CC2500 on host %d, CS pin %d", dev->bus->host,
           dev->cs_pin);
  return ESP_OK;
}

esp_err_t cc2500_deinit(cc2500_t* dev) {
  if (dev == NULL || dev->handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret = spi_bus_remove_device(dev->handle);
  if (ret == ESP_OK) {
    dev->handle = NULL;
  }
  return ret;
}

esp_err_t cc2500_bus_deinit(cc2500_bus_t* bus) {
  esp_err_t ret = spi_bus_free(bus->host);
  return ret;
}

void app_main(void) {
  cc2500_bus_t bus1 = {
      .host = SPI2_HOST, .sclk_pin = 6, .mosi_pin = 7, .miso_pin = 2};

  cc2500_t dev1 = {.bus = &bus1, .cs_pin = 10, .clock_speed_hz = 5000000};

  cc2500_bus_init(&bus1);
  cc2500_init(&dev1);
}