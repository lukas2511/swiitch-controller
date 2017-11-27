void i2c_write(uint8_t addr, uint8_t* buf, int len);
int i2c_read(uint8_t addr, uint8_t* buf, int len);

void init_i2c(void);
