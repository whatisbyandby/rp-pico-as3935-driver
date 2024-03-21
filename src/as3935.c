#include "as3935.h"
#include <stdio.h>

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

void print_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        // Use bitwise AND to check if the i-th bit is set
        if (byte & (1 << i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n"); // Move to a new line after printing all bits
}




as3935_error_t as3935_init(as3935_t *as3935)
{
    if (as3935->spi == NULL)
    {
        return AS3935_ERROR;
    }
    return AS3935_OK;
}

as3935_error_t as3935_write_register(as3935_t *as3935, uint8_t addr, uint8_t value)
{

    uint8_t write_buffer[2] = {addr | AS3935_WRITE_MASK, value};
    cs_select();
    bool success = spi_write_blocking(as3935->spi, write_buffer, 2) == 2;
    cs_deselect();
    if (!success)
    {
        return AS3935_ERROR;
    }
    return AS3935_OK;
}

as3935_error_t as3935_read_register(as3935_t *as3935, uint8_t addr, uint8_t *value)
{
    uint8_t write_buffer[1] = {addr | AS3935_READ_MASK};
    cs_select();
    bool success = spi_write_blocking(as3935->spi, write_buffer, 1) == 1;
    success = success | spi_read_blocking(as3935->spi, 0, value, 1) == 1;
    cs_deselect();
    if (!success)
    {
        return AS3935_ERROR;
    }
    return AS3935_OK;
}

as3935_error_t as3935_set_to_defaults(as3935_t *as3935)
{
    as3935_write_register(as3935, REG_PRESET_DEFAULT, DIRECT_COMMAND);

}

as3935_error_t as3935_set_afe(as3935_t *as3935, uint8_t new_afe)
{
    uint8_t current_value = 0x00;
    as3935_error_t err = as3935_read_register(as3935, 0x00, &current_value);
    // Clear bits 1-5 of the current value
    current_value &= ~(0x3E); // 0b00111110
    
    // Set bits 1-5 according to the input settings (ensure only bits 1-5 of settings are used)
    current_value |= (new_afe & 0x3E); // Apply new settings to bits 1-5

    as3935_write_register(as3935, 0x00, new_afe);
}

as3935_error_t as3935_set_noise_floor(as3935_t *as3935, uint8_t noise_floor)
{
    uint8_t current_value = 0x00;
    as3935_error_t err = as3935_read_register(as3935, REG_NF_LEV_WDTH, &current_value);
    
    // Clear bits 4-6 in the register
    // Mask for clearing bits 4-6: ~(0x7 << 3) = 0xC7
    current_value &= ~(0x7 << 4);

    // Set the value in bits 4-6
    // Shift the value to align with bits 4-6 and then set these bits in the register
    current_value |= (noise_floor << 4);

    as3935_write_register(as3935, REG_NF_LEV_WDTH, current_value);
}

as3935_error_t as3935_set_watchdog_threshold(as3935_t *as3935, uint8_t watchdog_threshold){
    // The watchdog threshold is bits 0-3 of register 0x01
    uint8_t current_value = 0x00;
    as3935_error_t err = as3935_read_register(as3935, REG_NF_LEV_WDTH, &current_value);

    if (err != AS3935_OK) return err;

    // Clear bits 0-3 of the current value
    current_value &= ~(0x0F); // 0b00001111
    // Set bits 0-3 according to the input settings (ensure only bits 0-3 of settings are used)
    current_value |= (watchdog_threshold & 0x0F); // Apply new settings to bits 0-3

    err = as3935_write_register(as3935, REG_NF_LEV_WDTH, current_value);

    if (err != AS3935_OK) return err;

    return AS3935_OK;
}

as3935_error_t as3935_power_down(as3935_t *as3935)
{

    // Set only bit zero to 1
    uint8_t current_reg = 0x00;
    as3935_error_t err = as3935_read_register(as3935, REG_AFE_GB_PWD, &current_reg);

    // check for an error
    if (err != AS3935_OK) return err;

    current_reg |= POWER_DOWN;

    err = as3935_write_register(as3935, REG_AFE_GB_PWD, current_reg);

    // check for an error
    if (err != AS3935_OK) return err;

    return AS3935_OK;
}

as3935_error_t as3935_power_up(as3935_t *as3935)
{
    // set only bit zero to 0
    uint8_t current_reg = 0x00;
    as3935_error_t err = as3935_read_register(as3935, REG_AFE_GB_PWD, &current_reg);

    // check for an error
    if (err != AS3935_OK) return err;

    current_reg &= ~POWER_DOWN;

    err = as3935_write_register(as3935, REG_AFE_GB_PWD, current_reg);

    // check for an error
    if (err != AS3935_OK) return err;

    return AS3935_OK;
}

as3935_error_t as3935_get_distance_km(as3935_t *as3935, uint8_t *distance_km)
{
    uint8_t reg_value = 0x00;
    as3935_error_t err = as3935_read_register(as3935, REG_DISTANCE, &reg_value);
    uint8_t reading = reg_value & DISTANCE_KM_MASK;

    if (reading == DISTANCE_OUT_OF_RANGE || reading == DISTANCE_NO_READING)
    {
        *distance_km = 0;
        return AS3935_ERROR;
    }
    else
    {
        *distance_km = reading;
        return AS3935_OK;
    }
}

as3935_error_t as3935_print_registers(as3935_t *as3935)
{
    uint8_t value = 0;
    for(int i = 0; i < 0x09; i++) {
        as3935_read_register(as3935, i, &value);
        printf("Register 0x%02x: 0x%02X\n", i, value);
    }
}
