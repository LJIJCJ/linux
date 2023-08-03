/**
 * SPDX-License-Identifier: GPL-2.0-only
 * 
 * @File Name: es9356.c
 * @brief  
 * @author Edgar Lai <laichengjie@everest-semi.com>
 * @Version : 1.0
 * @Create Date : 2023-08-01
 * 
 * @copyright Copyright (c) 2023 Everest Semiconductor, Ltd.
 */
#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/of_irq.h>
#include <linux/pm_runtime.h>
#include <linux/soundwire/sdw.h>
#include <linux/soundwire/sdw_registers.h>
#include <linux/soundwire/sdw_type.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/sdw.h>
#include <sound/soc.h>

#include "es9356.h"

// Reg Region
/**
 * @brief  debug mode, assert all reg are readable
 * @param  dev: 
 * @param  reg: 
 * @return true: 
 * @return false: 
 */
static bool es9356_readable_register(struct device *dev, unsigned int reg)
{
	return true;
}

/**
 * @brief  
 * @param  dev: 
 * @param  reg: 
 * @return true: 
 * @return false: 
 */
static bool es9356_volatile_register(struct device *dev, unsigned int reg)
{
	return true;
}

static int es9356_sdw_read(void *context, unsigned int reg, unsigned int *val)
{
	struct device *dev = context;
	struct rt711_priv *rt711 = dev_get_drvdata(dev);
	unsigned int sdw_data_3, sdw_data_2, sdw_data_1, sdw_data_0;
	unsigned int reg2 = 0, reg3 = 0, reg4 = 0, mask, nid, val2;
	unsigned int is_hda_reg = 1, is_index_reg = 0;
	int ret;

	if (reg > 0xffff)
		is_index_reg = 1;

	mask = reg & 0xf000;

	if (is_index_reg) { /* index registers */
		val2 = reg & 0xff;
		reg = reg >> 8;
		nid = reg & 0xff;
		ret = regmap_write(rt711->sdw_regmap, reg, 0);
		if (ret < 0)
			return ret;
		reg2 = reg + 0x1000;
		reg2 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2, val2);
		if (ret < 0)
			return ret;

		reg3 = RT711_PRIV_DATA_R_H | nid;
		ret = regmap_write(rt711->sdw_regmap, reg3,
				   ((*val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		reg4 = reg3 + 0x1000;
		reg4 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg4, (*val & 0xff));
		if (ret < 0)
			return ret;
	} else if (mask == 0x3000) {
		reg += 0x8000;
		ret = regmap_write(rt711->sdw_regmap, reg, *val);
		if (ret < 0)
			return ret;
	} else if (mask == 0x7000) {
		reg += 0x2000;
		reg |= 0x800;
		ret = regmap_write(rt711->sdw_regmap, reg,
				   ((*val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		reg2 = reg + 0x1000;
		reg2 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2, (*val & 0xff));
		if (ret < 0)
			return ret;
	} else if ((reg & 0xff00) == 0x8300) { /* for R channel */
		reg2 = reg - 0x1000;
		reg2 &= ~0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2,
				   ((*val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		ret = regmap_write(rt711->sdw_regmap, reg, (*val & 0xff));
		if (ret < 0)
			return ret;
	} else if (mask == 0x9000) {
		ret = regmap_write(rt711->sdw_regmap, reg,
				   ((*val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		reg2 = reg + 0x1000;
		reg2 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2, (*val & 0xff));
		if (ret < 0)
			return ret;
	} else if (mask == 0xb000) {
		ret = regmap_write(rt711->sdw_regmap, reg, *val);
		if (ret < 0)
			return ret;
	} else {
		ret = regmap_read(rt711->sdw_regmap, reg, val);
		if (ret < 0)
			return ret;
		is_hda_reg = 0;
	}

	if (is_hda_reg || is_index_reg) {
		sdw_data_3 = 0;
		sdw_data_2 = 0;
		sdw_data_1 = 0;
		sdw_data_0 = 0;
		ret = regmap_read(rt711->sdw_regmap, RT711_READ_HDA_3,
				  &sdw_data_3);
		if (ret < 0)
			return ret;
		ret = regmap_read(rt711->sdw_regmap, RT711_READ_HDA_2,
				  &sdw_data_2);
		if (ret < 0)
			return ret;
		ret = regmap_read(rt711->sdw_regmap, RT711_READ_HDA_1,
				  &sdw_data_1);
		if (ret < 0)
			return ret;
		ret = regmap_read(rt711->sdw_regmap, RT711_READ_HDA_0,
				  &sdw_data_0);
		if (ret < 0)
			return ret;
		*val = ((sdw_data_3 & 0xff) << 24) |
		       ((sdw_data_2 & 0xff) << 16) |
		       ((sdw_data_1 & 0xff) << 8) | (sdw_data_0 & 0xff);
	}

	if (is_hda_reg == 0)
		dev_dbg(dev, "[%s] %04x => %08x\n", __func__, reg, *val);
	else if (is_index_reg)
		dev_dbg(dev, "[%s] %04x %04x %04x %04x => %08x\n", __func__,
			reg, reg2, reg3, reg4, *val);
	else
		dev_dbg(dev, "[%s] %04x %04x => %08x\n", __func__, reg, reg2,
			*val);

	return 0;
}

static int es9356_sdw_write(void *context, unsigned int reg, unsigned int val)
{
	struct device *dev = context;
	struct rt711_priv *rt711 = dev_get_drvdata(dev);
	unsigned int reg2 = 0, reg3, reg4, nid, mask, val2;
	unsigned int is_index_reg = 0;
	int ret;

	if (reg > 0xffff)
		is_index_reg = 1;

	mask = reg & 0xf000;

	if (is_index_reg) { /* index registers */
		val2 = reg & 0xff;
		reg = reg >> 8;
		nid = reg & 0xff;
		ret = regmap_write(rt711->sdw_regmap, reg, 0);
		if (ret < 0)
			return ret;
		reg2 = reg + 0x1000;
		reg2 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2, val2);
		if (ret < 0)
			return ret;

		reg3 = RT711_PRIV_DATA_W_H | nid;
		ret = regmap_write(rt711->sdw_regmap, reg3,
				   ((val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		reg4 = reg3 + 0x1000;
		reg4 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg4, (val & 0xff));
		if (ret < 0)
			return ret;
		is_index_reg = 1;
	} else if (reg < 0x4fff) {
		ret = regmap_write(rt711->sdw_regmap, reg, val);
		if (ret < 0)
			return ret;
	} else if (reg == RT711_FUNC_RESET) {
		ret = regmap_write(rt711->sdw_regmap, reg, val);
		if (ret < 0)
			return ret;
	} else if (mask == 0x7000) {
		ret = regmap_write(rt711->sdw_regmap, reg, ((val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		reg2 = reg + 0x1000;
		reg2 |= 0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2, (val & 0xff));
		if (ret < 0)
			return ret;
	} else if ((reg & 0xff00) == 0x8300) { /* for R channel */
		reg2 = reg - 0x1000;
		reg2 &= ~0x80;
		ret = regmap_write(rt711->sdw_regmap, reg2,
				   ((val >> 8) & 0xff));
		if (ret < 0)
			return ret;
		ret = regmap_write(rt711->sdw_regmap, reg, (val & 0xff));
		if (ret < 0)
			return ret;
	}

	if (reg2 == 0)
		dev_dbg(dev, "[%s] %04x <= %04x\n", __func__, reg, val);
	else if (is_index_reg)
		dev_dbg(dev, "[%s] %04x %04x %04x %04x <= %04x %04x\n",
			__func__, reg, reg2, reg3, reg4, val2, val);
	else
		dev_dbg(dev, "[%s] %04x %04x <= %04x\n", __func__, reg, reg2,
			val);

	return 0;
}

const struct regmap_config es9356_regmap = {
	.name = "sdw",
	.reg_bits = 32,
	.val_bits = 8,

	.readable_reg = es9356_readable_register,
	.volatile_reg = es9356_volatile_register,

	.reg_defaults = es9356_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(es9356_reg_defaults),

	.reg_read = es9356_sdw_read,
	.reg_write = es9356_sdw_write,

	.max_register = 0xff01,
	.cache_type = REGCACHE_NONE,
	.use_single_read = true,
	.use_single_write = true,
};

// DAPM Route Region
static const struct snd_soc_dapm_route es9356_sdw_audio_map[] = {
	/* Playback Path */
	{ "HP", NULL, "MIXER" },
	{ "MIXER", NULL, "DACSRC" },
	{ "DACSRC", NULL, "Playback" },

	/* Capture Path */
	{ "ADCSRC", NULL, "HS" },
	{ "Capture", NULL, "ADCSRC" },
};

// DAI Region
static int es8356_sdw_dai_startup(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	return 0;
}

static void es9356_sdw_dai_shutdown(struct snd_pcm_substream *substream,
				    struct snd_soc_dai *dai)
{
	return;
}

static int es9356_sdw_dai_hw_params(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *params,
				    struct snd_soc_dai *dai)
{
	return 0;
}

static int es9356_sdw_dai_prepare(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	return 0;
}

static int es9356_sdw_dai_hw_free(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	return 0;
}

int es9356_mute_stream(struct snd_soc_dai *dai, int mute, int stream)
{
	return 0;
}
EXPORT_SYMBOL_NS_GPL(es9356_mute_stream, SND_SOC_ES9356_CORE);

static int es9356_sdw_dai_set_sdw_stream(struct snd_soc_dai *dai,
					 void *sdw_stream, int direction)
{
	return 0;
}

static const struct snd_soc_dai_ops es9356_sdw_dai_ops = {
	.startup = es8356_sdw_dai_startup,
	.shutdown = es9356_sdw_dai_shutdown,
	.hw_params = es9356_sdw_dai_hw_params,
	.prepare = es9356_sdw_dai_prepare,
	.hw_free = es9356_sdw_dai_hw_free,
	.mute_stream = es9356_mute_stream,
	.set_stream = es9356_sdw_dai_set_sdw_stream,
};

static struct snd_soc_dai_driver es9356_sdw_dai = {
	.name = "es9356-sdw",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		/* Restrict which rates and formats are supported */
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 1,
		/* Restrict which rates and formats are supported */
		.rates = SNDRV_PCM_RATE_8000_96000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
	},
	.symmetric_rate = 1,
	.ops = &es9356_sdw_dai_ops,
};

// Slave Option Interface Region
/**
 * @brief  
 * @param  peripheral: 
 * @return int: 
 */
static int es9356_sdw_read_prop(struct sdw_slave *peripheral)
{
	return 0;
}

static int es9356_sdw_update_status(struct sdw_slave *peripheral,
				    enum sdw_slave_status status)
{
	return 0;
}

static int es9356_sdw_bus_config(struct sdw_slave *peripheral,
				 struct sdw_bus_params *params)
{
	return 0;
}

static int es9356_sdw_port_prep(struct sdw_slave *slave,
				struct sdw_prepare_ch *prepare_ch,
				enum sdw_port_prep_ops state)
{
	return 0;
}

static const struct sdw_slave_ops es9356_sdw_ops = {
	.read_prop = es9356_sdw_read_prop,
	.update_status = es9356_sdw_update_status,
	.bus_config = es9356_sdw_bus_config,
	.port_prep = es9356_sdw_port_prep,
};

// Power Management Region

int es9356_suspend(struct device *dev)
{
	return 0;
}

static int __maybe_unused es9356_sdw_resume(struct device *dev)
{
	return 0;
}

static int __maybe_unused es9356_sdw_runtime_suspend(struct device *dev)
{
	return 0;
}

/**
 * @brief 
 * 
 * @param dev 
 * @return int 
 */
static int __maybe_unused es9356_sdw_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops es9356_sdw_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(es9356_suspend, es9356_sdw_resume)
		SET_RUNTIME_PM_OPS(es9356_sdw_runtime_suspend,
				   es9356_sdw_runtime_resume, NULL)
};

// Module Region

static int es9356_sdw_probe(struct sdw_slave *peripheral,
			    const struct sdw_device_id *id)
{
	struct snd_soc_component_driver *component_drv;
	struct device *dev = &peripheral->dev;
	struct es9356_private *es9356;
	struct regmap_config *regmap_conf;
	struct regmap *regmap;
	int irq, ret;

	// allocate driver memory
	es9356 = devm_kzalloc(dev, sizeof(*es9356), GFP_KERNEL);
	if (!es9356)
		return -ENOMEM;
	// get gpio resource
	if (has_acpi_companion(dev))
		irq = acpi_dev_gpio_irq_get(ACPI_COMPANION(dev), 0);
	else
		irq = of_irq_get(dev->of_node, 0);

	if (irq == -ENOENT)
		irq = 0;
	else if (irq < 0)
		return dev_err_probe(dev, irq, "Failed to get IRQ\n");

	regmap_conf = devm_kmemdup(dev, &es9356_regmap, sizeof(es9356_regmap),
				   GFP_KERNEL);
	if (!regmap_conf)
		return -ENOMEM;

	return 0;
}

static int es9356_sdw_remove(struct sdw_slave *peripheral)
{
	return 0;
}

static const struct sdw_device_id es9356_sdw_id[] = {
	SDW_SLAVE_ENTRY(0x04b3, 0x9356, 0),
	{},
};
MODULE_DEVICE_TABLE(sdw, es9356_sdw_id);

static struct sdw_driver es9356_sdw_driver = {
        .driver = {
                   .name = "es9356-sdw",
                   .pm = &es9356_sdw_pm, //power manage
                   },
        .probe = es9356_sdw_probe,
        .remove = es9356_sdw_remove,
        .ops = &es9356_sdw_ops,
        .id_table = es9356_sdw_id,
};

module_sdw_driver(es9356_sdw_driver);

MODULE_DESCRIPTION("ASoC ES9356 SoundWire driver");
MODULE_AUTHOR("Edgar Lai <laichengjie@everest-semi.com>");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(SND_SOC_ES9356_CORE);