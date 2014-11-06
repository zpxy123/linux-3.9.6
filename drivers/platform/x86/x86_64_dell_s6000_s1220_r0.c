/**
 * Dell S6000 Platform Support.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

#include <linux/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/dmi.h>


/*************************************************************
 *
 * I2C Bus 0 on the S6000 is muxed via gpio1 and gpio2.
 *
 ************************************************************/
static const unsigned s6000_gpiomux_gpios[] = {
    1, 2
};

static const unsigned s6000_gpiomux_values[] = {
    0, 1, 2, 3
};

static struct i2c_mux_gpio_platform_data s6000_i2cmux_data = {
    /*
     * i2c Bus 0
     */
    .parent     = 0,

    /*
     * Start the bus numbers at 10. The first digit
     * will represent the different bus numbers based
     * the gpio selector (00, 01, 10, 11):
     *
     * i2c-10 --> i2c-0, gpios = 00
     * i2c-11 --> i2c-0, gpios = 01
     * i2c-12 --> i2c-0, gpios = 10
     * i2c-13 --> i2c-0, gpios = 11
     */
    .base_nr    = 10,

    .values     = s6000_gpiomux_values,
    .n_values   = ARRAY_SIZE(s6000_gpiomux_values),
    .gpios      = s6000_gpiomux_gpios,
    .n_gpios    = ARRAY_SIZE(s6000_gpiomux_gpios),
    .idle       = 0,
};

static struct platform_device s6000_i2cmux = {
    .name = "i2c-mux-gpio",
    .id   = 12,
    .dev  = {
        .platform_data  = &s6000_i2cmux_data,
    },
};

/*************************************************************
 *
 * Sensors on i2c-11 (See mux data above).
 *
 ************************************************************/
static struct i2c_board_info s6000_i2c_11_board_info[] = {
    { I2C_BOARD_INFO("lm75", 0x4c) },
    { I2C_BOARD_INFO("lm75", 0x4d) },
    { I2C_BOARD_INFO("lm75", 0x4e) },
    { I2C_BOARD_INFO("ltc4215", 0x42) },
    { I2C_BOARD_INFO("ltc4215", 0x40) },
    { I2C_BOARD_INFO("max6620", 0x29) },
    { I2C_BOARD_INFO("max6620", 0x2A) },
    { I2C_BOARD_INFO("24c02", 0x51) },
    { I2C_BOARD_INFO("24c02", 0x52) },
    { I2C_BOARD_INFO("24c02", 0x53) },
};

static int __init x86_64_dell_s6000_s1220_r0_init(void)
{
    int i;
    int rv = 0;
    char const *vendor, *product;
    struct i2c_adapter * i2ca;

    vendor  = dmi_get_system_info(DMI_SYS_VENDOR);
    product = dmi_get_system_info(DMI_PRODUCT_NAME);

    if(strcmp(vendor, "Dell Inc") ||
       (strcmp(product, "S6000 (SI)") && strcmp(product, "S6000-ON") &&
        strcmp(product, "S6000-ON (SI)"))) {
        /* Not the S6000 */
        return -ENODEV;
    }

    /**
     * Register the GPIO mux for bus 0.
     */
    rv = platform_device_register(&s6000_i2cmux);
    if(rv < 0) {
        pr_err("%s: platform_device_register() failed: %d", __FUNCTION__, rv);
        return rv;
    }


    /**
     * Register I2C devices on new buses
     */
    i2ca = i2c_get_adapter(11);
    for(i = 0; i < ARRAY_SIZE(s6000_i2c_11_board_info); i++) {
        if(i2c_new_device(i2ca, s6000_i2c_11_board_info+i) == NULL) {
            pr_err("%s: i2c_new_device for bus 11:0x%x failed.",
                   __FUNCTION__, s6000_i2c_11_board_info[i].addr);
        }
    }

    return 0;

}

static void __exit x86_64_dell_s6000_s1220_r0_cleanup(void)
{
    platform_device_unregister(&s6000_i2cmux);
}

module_init(x86_64_dell_s6000_s1220_r0_init);
module_exit(x86_64_dell_s6000_s1220_r0_cleanup);

MODULE_AUTHOR("Big Switch Networks (support@bigswitch.com)");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Dell S6000");
MODULE_LICENSE("GPL");

