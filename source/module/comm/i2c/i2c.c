// Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten
/***
 * @file i2c.c
 * @copyright Urheberrecht 2018-2020 ESoPe GmbH, Alle Rechte vorbehalten. Released under an Apache 2.0 license.
 **/

#include "i2c.h"
#if MODULE_ENABLE_COMM_I2C
#include "../dbg.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Sets the level on the SDA output pin.
//#define SDA(val)			PORT1.PODR.BIT.B7 = (val)
#define SDA(val) mcu_io_set(h->sda, (val))
/// Returns the level on the SDA input pin.
//#define SDA_GET()			PORT1.PIDR.BIT.B7
#define SDA_GET(val) mcu_io_get(h->sda)
/// Sets the level on the SCL output pin.
//#define SCL(val)			PORT1.PODR.BIT.B6 = (val)
#define SCL(val) mcu_io_set(h->scl, (val))
/// Delays a clock period for each clock cycle.
//#define	I2C_DELAY()			_nop();
#define	I2C_DELAY()	_timer_flag = true; mcu_timer_start(_timer); while(_timer_flag);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Since the i2c read and write function is blocking, one shared timer for all I2C interfaces is used.
/// Is initialized during i2c_init.
static mcu_timer_t _timer = NULL;
/// Flag is set before the timer is started and cleared in the timer used to wait between each clock period.
static volatile bool _timer_flag = false;
/// Flag indicating if the internal I2C data is initialized.
/// FIXME: Should be set for each physical I2C interface.
static bool _initialized_internal = false;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Manual I2C implementation: Sets the I2C start condition.
 * SDA and SCL are high in the beginning and SDA is set to low afterwards.
 *
 * @param h			Pointer to the I2C interface context structure.
 */
static void _i2c_start(i2c_t* h);
/**
 * Manual I2C implementation: Sets the I2C stop condition.
 * SDA is low in the beginning, then the SCL and SDA are set to high afterwards.
 * @param h			Pointer to the I2C interface context structure.
 */
static void _i2c_stop(i2c_t* h);
/**
 * Manual I2C implementation: Writes a single byte on the I2C interface and reads the ack bit.
 * @param h			Pointer to the I2C interface context structure.
 * @param d			Byte to send via I2C.
 * @return			true: Ack was set.\n
 * 					false: Ack was not set -> Writing was not successful.
 */
static bool _i2c_write_byte(i2c_t* h, uint8_t d);
/**
 * Manual I2C implementation: Reads a byte from the I2C interface and sets the ack bit.
 * @param h			Pointer to the I2C interface context structure.
 * @param ack		In master mode the ack flag is set to false when reading the last byte.
 * @return			Byte read from the I2C interface.
 */
static uint8_t _i2c_read_byte(i2c_t* h, bool ack);
/**
 * Manual I2C implementation: Timer is triggered for each I2C clock cycle. Clears the timer flag and stops the timer.
 * @param h			Pointer to the I2C interface context structure.
 */
static void _i2c_timer(i2c_t* h);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
void i2c_init(i2c_t* h, uint8_t num, MCU_IO_PIN sda, MCU_IO_PIN scl)
{
#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	h->i2c = mcu_i2c_init(num, sda, scl);
#else
	h->i2c = NULL;
#endif
	h->sda = sda;
	h->scl = scl;

	if(h->i2c == NULL && !_initialized_internal)
	{
		if(_timer == NULL)
		{
			_timer = mcu_timer_init(MCU_INT_LVL_MED, 100000, (void(*)(void*))_i2c_timer, h, false);
		}

		mcu_io_set_dir(h->scl, MCU_IO_DIR_OUT);
		SCL(1);
		mcu_io_set_dir(h->sda, MCU_IO_DIR_OUT);
		mcu_io_set_pullup(h->sda, true);
		SDA(1);

		_initialized_internal = true;
	}
}

void i2c_free(i2c_t* h)
{
#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	if(h->i2c)
	{
		mcu_i2c_free(h->i2c);
		h->i2c = NULL;
	}
#endif
	h->scl = PIN_NONE;
	h->sda = PIN_NONE;
}

void i2c_set_frq(i2c_t* h, uint32_t frequency)
{
	if(h->i2c == NULL)
	{
		mcu_timer_set_frq(_timer, frequency << 1 /*= frequency * 2*/);
		h->frequency = mcu_timer_get_frq(_timer);
	}
#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	else
	{
		mcu_i2c_set_frq(h->i2c, frequency);
		h->frequency = mcu_i2c_get_frq(h->i2c);
	}
#endif
//	h->wait_time = (1000000 / (2 * frequency));
//	h->frequency = 1000000 / 2 * (uint32_t)h->wait_time;
	dbg_printf(DBG_STRING, "FRQ=%d\n", (uint32_t)h->frequency);
}

uint32_t i2c_get_frq(i2c_t* h)
{
	return h->frequency;
}

void i2c_set_address(i2c_t* h, uint8_t address)
{
	h->addr = address << 1;
#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	if(h->i2c != NULL)
		mcu_i2c_set_address(h->i2c, address);
#endif
}

bool i2c_wr(i2c_t* h, uint8_t* wbuf, size_t wlen, uint8_t* rbuf, size_t rlen)
{
//	uint32_t timestamp = system_get_tick_count();
	uint8_t i;

	if(h == NULL || (wlen > 0 && wbuf == NULL) || (rlen > 0 && rbuf == NULL) || (wlen == 0 && rlen == 0))
		return false;

#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	if(h->i2c != NULL)
		return mcu_i2c_wr(h->i2c, wbuf, wlen, rbuf, rlen);
#endif

	if(mcu_timer_get_frq(_timer) != h->frequency)
		mcu_timer_set_frq(_timer, h->frequency);

	// Initiate Start Condition
	_i2c_start(h);

	// If data for writing available, write it first
	if(wlen > 0)
	{
		// Send address
		if(!_i2c_write_byte(h, h->addr))
		{
			DBG_ERROR("NAK ADR!\n");
			_i2c_stop(h);
			return false;
		}

		// Write buffered data
		for(i = 0; i < wlen; i++)
		{
			if(!_i2c_write_byte(h, *wbuf++))
			{
				DBG_ERROR("NAK!\n");
				_i2c_stop(h);
				return false;
			}
		}
	}

	// If data should be read, read it
	if(rlen > 0)
	{
		// If data was written first, re-enable start condition!
		if(wlen > 0)
			_i2c_start(h);

		// Send address
		_i2c_write_byte(h, h->addr | 1);

		// Write buffered data
		for(i = 0; i < rlen; i++)
			*rbuf++ = _i2c_read_byte(h, (i < (rlen - 1)));
	}

	// Initiate Stop Condition
	_i2c_stop(h);

//	dbg_printf(DBG_STRING, "[%ums / %dHz] i2c_wr(%d, %d)\n", system_get_tick_count() - timestamp, mcu_timer_get_frq(_timer), wlen, rlen);

	return true;
}

bool i2c_wwr(i2c_t* h, uint8_t* wbuf, size_t wlen, uint8_t* w2buf, size_t w2len, uint8_t* rbuf, size_t rlen)
{
//	uint32_t timestamp = system_get_tick_count();
	uint8_t i;

	if(h == NULL || (wlen > 0 && wbuf == NULL) || (rlen > 0 && rbuf == NULL) || (wlen == 0 && rlen == 0))
		return false;

#if MCU_PERIPHERY_DEVICE_COUNT_I2C
	if(h->i2c != NULL)
		return mcu_i2c_wwr(h->i2c, wbuf, wlen, w2buf, w2len, rbuf, rlen);
#endif

	if(mcu_timer_get_frq(_timer) != h->frequency)
		mcu_timer_set_frq(_timer, h->frequency);

	// Initiate Start Condition
	_i2c_start(h);

	// If data for writing available, write it first
	if(wlen > 0)
	{
		// Send address
		if(!_i2c_write_byte(h, h->addr))
		{
			DBG_ERROR("NAK ADR!\n");
			_i2c_stop(h);
			return false;
		}

		// Write buffered data
		for(i = 0; i < wlen; i++)
		{
			if(!_i2c_write_byte(h, *wbuf++))
			{
				DBG_ERROR("NAK!\n");
				_i2c_stop(h);
				return false;
			}
		}
	}

	// If data for writing available, write it first
	if(w2len > 0)
	{
		// Send address
		if(!_i2c_write_byte(h, h->addr))
		{
			DBG_ERROR("NAK ADR!\n");
			_i2c_stop(h);
			return false;
		}

		// Write buffered data
		for(i = 0; i < w2len; i++)
		{
			if(!_i2c_write_byte(h, *w2buf++))
			{
				DBG_ERROR("NAK!\n");
				_i2c_stop(h);
				return false;
			}
		}
	}

	// If data should be read, read it
	if(rlen > 0)
	{
		// If data was written first, re-enable start condition!
		if(wlen > 0)
			_i2c_start(h);

		// Send address
		_i2c_write_byte(h, h->addr | 1);

		// Write buffered data
		for(i = 0; i < rlen; i++)
			*rbuf++ = _i2c_read_byte(h, (i < (rlen - 1)));
	}

	// Initiate Stop Condition
	_i2c_stop(h);

//	dbg_printf(DBG_STRING, "[%ums / %dHz] i2c_wr(%d, %d)\n", system_get_tick_count() - timestamp, mcu_timer_get_frq(_timer), wlen, rlen);

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/* Initiate start condition */
static void _i2c_start(i2c_t* h)
{
//	dbg_printf(DBG_STRING, "i2c_start()\n");
	mcu_io_set_dir(h->sda, MCU_IO_DIR_OUT);
	SDA(1);
	I2C_DELAY();
	SCL(1);
	I2C_DELAY();
	SDA(0);
	I2C_DELAY();
}

/* Initiate stop condition */
static void _i2c_stop(i2c_t* h)
{
	SDA(0);
	I2C_DELAY();
	SCL(1);
	I2C_DELAY();
	SDA(1);
	I2C_DELAY();
//	dbg_printf(DBG_STRING, "_i2c_stop()\n");
}

static const uint8_t _bits_lookup[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static bool _i2c_write_byte(i2c_t* h, uint8_t d)
{
	uint8_t i;

	SCL(0);
	I2C_DELAY();

	for (i = 0; i < 8; i++)
	{
		SDA((d & _bits_lookup[i]) == _bits_lookup[i]);
//		I2C_DELAY();
		SCL(1);
		I2C_DELAY();

		SCL(0);
		I2C_DELAY();
	}

	SDA(1);
	I2C_DELAY();

	mcu_io_set_dir(h->sda, MCU_IO_DIR_IN);			/* Release the RTC_SDA line */
	SCL(1);
	I2C_DELAY();

	i = SDA_GET();	/* get RTC_SDA status, RTC_ACK or nrtc_ACK */

	SCL(0);
	I2C_DELAY();
	SDA(0);
	mcu_io_set_dir(h->sda, MCU_IO_DIR_OUT);

//	dbg_printf(DBG_STRING, "i2c_write_byte(%02x) -> %d\n", (uint32_t)d, (uint32_t)i);

	return (i == 0);
}

static uint8_t _i2c_read_byte(i2c_t* h, bool ack)
{
	uint8_t i = 0, d = 0;

	SDA(1);	/* Release the RTC_SDA line */
	I2C_DELAY();

	SCL(0);
	I2C_DELAY();

	mcu_io_set_dir(h->sda, MCU_IO_DIR_IN);

	for (i = 0; i < 8; i++)	/* read the msb first */
	{
		SCL(1);
		I2C_DELAY();

		if(SDA_GET())
			d |= _bits_lookup[i];

		SCL(0);
		I2C_DELAY();
	}

	mcu_io_set_dir(h->sda, MCU_IO_DIR_OUT);

	SDA(!ack);		 /* 1 = RTC_NACK, 0 = RTC_ACK */
	I2C_DELAY();

	SCL(1);
	I2C_DELAY();

	SCL(0);
	I2C_DELAY();

	SDA(1);		 /* Release the RTC_SDA line */
	I2C_DELAY();

//	dbg_printf(DBG_STRING, "_i2c_read_byte() -> %02x\n", (uint32_t)d);

	return d;
}

static void _i2c_timer(i2c_t* h)
{
	_timer_flag = false;
	mcu_timer_stop(_timer);
}

#endif