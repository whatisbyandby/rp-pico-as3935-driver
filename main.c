#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/spi.h"
#include "as3935.h"

static inline void cs_select()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}

volatile uint8_t interrupt_reason = 0x00;

static inline void interrupt_handler(uint gpio, uint32_t events)
{
    uint8_t interrupt_reason = 0x00;
    uint8_t write_buffer = REG_INTERRUPT | 0x40;
    cs_select();
    spi_write_blocking(SPI_PORT, &write_buffer, 1);
    spi_read_blocking(SPI_PORT, 0, &interrupt_reason, 1);
    cs_deselect();
    printf("Interrupt Register: 0x%02X\n", interrupt_reason);
}

int main()
{

    stdio_init_all();

    // initalize the SPI interface
    spi_init(SPI_PORT, 1000 * 1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Set up the interrupt pin
    gpio_set_irq_enabled_with_callback(PIN_INTR, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);

    as3935_t as3935 = {
        .spi = spi0,
        .cs_pin = 17,
    };

    as3935_set_to_defaults(&as3935);

    as3935_set_noise_floor(&as3935, 7);
    as3935_set_watchdog_threshold(&as3935, 2);

    as3935_print_registers(&as3935);

    while (true)
    {
        sleep_ms(5000);
    }
}