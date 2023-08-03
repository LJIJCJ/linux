#ifndef ES9356_H
#define ES9356_H

#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/soundwire/sdw.h>
#include <sound/jack.h>
#include <sound/soc-component.h>
#include <sound/soc-dai.h>

#define ES9356_PAGE_00 0x0000
#define ES9356_PAGE_01 0x0100
#define ES9356_PAGE_02 0x0200

#define ES9356_CHIP_ID 9356

/* Page 0x00 Global Registers*/

// Manufacture ID
#define ES9356_MANID_AB (ES9356_PAGE_00 + 0x51)
#define ES9356_MANID_CD (ES9356_PAGE_00 + 0x52)
// Part ID
#define ES9356_DEVID_AB (ES9356_PAGE_00 + 0x53)
#define ES9356_DEVID_CD (ES9356_PAGE_00 + 0x54)

// Codec Region
static const struct reg_default es9356_reg_defaults[] = {
	{ 0x40, 0x00 },
	{ 0x41, 0x00 },
	{ 0x42, 0x00 },
	{ 0x43, 0x00 },
	{ 0x44, 0x20 },
	{ 0x45, 0x01 },
	{ 0x46, 0x00 },
	{ 0x48, 0x00 },
	{ 0x49, 0x00 },
	{ 0x4C, 0x00 },
	{ 0x50, 0x20 },
	{ 0x51, 0x04 },
	{ 0x52, 0x93 },
	{ 0x53, 0x56 },
	{ 0x54, 0x00 },
	{ 0x55, 0x00 },
	{ 0x60, 0x00 },
	{ 0x70, 0x00 },
	{ 0x100, 0x00 },
	{ 0x101, 0x00 },
	{ 0x102, 0x00 },
	{ 0x103, 0x17 },
	{ 0x104, 0x00 },
	{ 0x105, 0x3F },
	{ 0x120, 0x00 },
	{ 0x122, 0x5F },
	{ 0x123, 0x00 },
	{ 0x124, 0x00 },
	{ 0x125, 0x00 },
	{ 0x126, 0x00 },
	{ 0x127, 0x00 },
	{ 0x130, 0x00 },
	{ 0x132, 0x00 },
	{ 0x133, 0x00 },
	{ 0x134, 0x00 },
	{ 0x135, 0x00 },
	{ 0x136, 0x00 },
	{ 0x137, 0x00 },
	{ 0x200, 0x00 },
	{ 0x201, 0x00 },
	{ 0x202, 0x20 },
	{ 0x203, 0x17 },
	{ 0x204, 0x00 },
	{ 0x205, 0x0F },
	{ 0x220, 0x00 },
	{ 0x222, 0x5F },
	{ 0x223, 0x00 },
	{ 0x224, 0x00 },
	{ 0x225, 0x00 },
	{ 0x225, 0x00 },
	{ 0x226, 0x00 },
	{ 0x227, 0x00 },
	{ 0x230, 0x00 },
	{ 0x231, 0x00 },
	{ 0x232, 0x00 },
	{ 0x233, 0x00 },
	{ 0x234, 0x00 },
	{ 0x235, 0x00 },
	{ 0x236, 0x00 },
	{ 0x237, 0x00 },

};

#define ES9356_NUM_SUPPLIES 5
/* Button Detect Level Sensitivities */
#define ES9356_NUM_BIASES 4

struct es9356_private {
	struct regmap *regmap;
	struct device *dev;
	struct regulator_bulk_data supplies[ES9356_NUM_SUPPLIES];
	struct gpio_desc *reset_gpio;
	struct completion pdn_done;
	struct snd_soc_jack *jack;
	struct sdw_slave *sdw_peripheral;
	struct mutex irq_lock;
	int devid;
	int irq;
	int pll_config;
	u32 sclk;
	u32 sample_rate;
	u32 bclk_ratio;
	u8 plug_state;
	u8 hs_type;
	u8 ts_inv;
	u8 ts_dbnc_rise;
	u8 ts_dbnc_fall;
	u8 btn_det_init_dbnce;
	u8 btn_det_event_dbnce;
	u8 bias_thresholds[ES9356_NUM_BIASES];
	u8 hs_bias_ramp_rate;
	u8 hs_bias_ramp_time;
	u8 hs_bias_sense_en;
	u8 stream_use;
	bool hp_adc_up_pending;
	bool suspended;
	bool init_done;
};

#endif //ES9356_H