#include <linux/i2c.h>


/* Defines for device identification */ 
#define I2C_BUS_AVAILABLE	1		/* The I2C Bus available on the raspberry */
#define SLAVE_DEVICE_NAME	"bh1750"	/* Device and Driver Name */
#define bh1750_SLAVE_ADDRESS	0x23		/* bh1750 I2C address */

#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CONTINUOUS_MEASUREMENT 0x10
#define Continuously_H_Resolution_Mode2 0x11
#define Continuously_L_Resolution_Mode 0x13

static struct i2c_adapter * bh1750_i2c_adapter = NULL;
static struct i2c_client * bh1750_i2c_client = NULL;

static const struct i2c_device_id bh1750_id[] = {
		{ SLAVE_DEVICE_NAME, 0 }, 
		{ }
};

MODULE_DEVICE_TABLE(i2c, bh1750_id);

static struct i2c_board_info bh1750_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, bh1750_SLAVE_ADDRESS)	
};

static int bh1750_init(struct i2c_client *client);
