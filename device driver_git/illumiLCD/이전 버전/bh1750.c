
static int bh1750_init(struct i2c_client *client)
{
	int ret;
    // Send the power-on command to the BH1750 sensor
    char cmd_power_on = BH1750_POWER_ON;
    ret = i2c_master_send(client, &cmd_power_on, 1);
    if (ret < 0) {
        printk(KERN_ERR "Error sending power-on command to BH1750 sensor\n");
        return ret;
    }

    // Set the measurement mode to high-resolution mode
    char cmd_set_mode = Continuously_L_Resolution_Mode;
    ret = i2c_master_send(client, &cmd_set_mode, 1);
    if (ret < 0) {
        printk(KERN_ERR "Error setting measurement mode of BH1750 sensor\n");
        return ret;
    }

    return 0;
}