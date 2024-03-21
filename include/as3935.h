#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_INTR 20

#define REG_AFE_GB_PWD 0x00
#define REG_NF_LEV_WDTH 0x01

#define REG_INTERRUPT 0x03
#define REG_DISTANCE 0x07

#define AS3935_WRITE_MASK 0x00
#define AS3935_READ_MASK 0x40

#define AS3935_INTERRUPT_MASK 0x0F

#define AFE_MASK 0x3E
#define AFE_SETTING_INDOOR 0x12
#define AFE_SETTING_OUTDOOR 0x0E

#define POWER_UP 0x00
#define POWER_DOWN 0x01

#define DIRECT_COMMAND 0x96

#define REG_PRESET_DEFAULT 0x3C

#define DISTANCE_KM_MASK 0x3F
#define DISTANCE_OUT_OF_RANGE 0x3F
#define DISTANCE_NO_READING 0x00

typedef enum
{
    AS3935_OK = 0,
    AS3935_ERROR = 1,
    AS3935_WRITE_ERROR = 2,
} as3935_error_t;

typedef struct
{
    spi_inst_t *spi;
    uint8_t cs_pin;

} as3935_t;

void print_binary(uint8_t byte);
as3935_error_t as3935_init(as3935_t *as3935);
as3935_error_t as3935_set_to_defaults(as3935_t *as3935);
as3935_error_t as3935_write_register(as3935_t *as3935, uint8_t reg, uint8_t value);
as3935_error_t as3935_read_register(as3935_t *as3935, uint8_t addr, uint8_t *value);
as3935_error_t as3935_set_afe(as3935_t *as3935, uint8_t afe);
as3935_error_t as3935_set_noise_floor(as3935_t *as3935, uint8_t noise_floor);
as3935_error_t as3935_set_watchdog_threshold(as3935_t *as3935, uint8_t watchdog_threshold);

as3935_error_t as3935_power_up(as3935_t *as3935);
as3935_error_t as3935_power_down(as3935_t *as3935);

as3935_error_t as3935_get_distance_km(as3935_t *as3935, uint8_t *distance_km);
as3935_error_t as3935_print_registers(as3935_t *as3935);

