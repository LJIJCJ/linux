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

static const struct sdw_slave_ops es9356_sdw_ops = {
	// .read_prop = es9356_sdw_read_prop,
	// .update_status = es9356_sdw_update_status,
	// .bus_config = es9356_sdw_bus_config,
	// .port_prep = es9356_sdw_port_prep,
};

int es9356_suspend(struct device *dev){
        return 0;
}

static int __maybe_unused es9356_sdw_resume(struct device *dev){
        return 0;
}

static int __maybe_unused es9356_sdw_runtime_suspend(struct device *dev){
        return 0;
}

static int __maybe_unused es9356_sdw_runtime_resume(struct device *dev){
        return 0;
}

static const struct dev_pm_ops es9356_sdw_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(es9356_suspend, es9356_sdw_resume)
	SET_RUNTIME_PM_OPS(es9356_sdw_runtime_suspend, es9356_sdw_runtime_resume, NULL)
};

static int es9356_sdw_probe(struct sdw_slave *peripheral, const struct sdw_device_id *id){
        return 0;
}

static int es9356_sdw_remove(struct sdw_slave *peripheral){
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