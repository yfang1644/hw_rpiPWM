#ifndef _RPII2C_H
#define _PRII2C_H


/* hardware related constants */
#define I2C1_BASE_OFFSET    0x804000

/* I2C contronl register */
#define I2C_CTL             0
#   define I2C_EN			(1<<15)		/* enable */
#	define I2C_START		(1<<7)		/* start transfer */
#	define I2C_CLEAR		(3<<4)		/* clear fifo */
#	define I2C_READ			(1<<0)		/* read packet transfer */

#define I2C_STATUS			1	/* status */
#   define I2C_RXD          (1<<5)		/* FIFO is to read */
#   define I2C_TXD          (1<<4)		/* FIFO is ready to transfer */
#   define I2C_DONE         (1<<1)		/* transfer complete */
#   define I2C_TA           (1<<0)		/* transfer active */

#define I2C_DLEN			2	/* data length */
#define I2C_SLAVE			3	/* slave address */
#define	I2C_FIFO			4   /* data fifo */
#define I2C_DIV				5   /* clock divisor, 150M/div, */
#define I2C_DELAY			6   /* data delay */
#define I2C_CLKT			7   /* clock stretch timeout */

int i2c_init(int div);
int i2c_write(int address, char word);
int i2c_stop(void);

#endif     /*  _RPII2C_H */
