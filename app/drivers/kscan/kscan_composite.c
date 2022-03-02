/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_kscan_composite

#include <device.h>
#include <drivers/kscan.h>
#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define MATRIX_NODE_ID DT_DRV_INST(0)
#define MATRIX_ROWS DT_PROP(MATRIX_NODE_ID, rows)
#define MATRIX_COLS DT_PROP(MATRIX_NODE_ID, columns)

struct kscan_composite_child_config {
    char *label;
    uint8_t row_offset;
    uint8_t column_offset;
};

#define CHILD_CONFIG(inst)                                                                         \
    {.label = DT_LABEL(DT_PHANDLE(inst, kscan)),                                                   \
     .row_offset = DT_PROP(inst, row_offset),                                                      \
     .column_offset = DT_PROP(inst, column_offset)},

const struct kscan_composite_child_config kscan_composite_children[] = {
    DT_FOREACH_CHILD(MATRIX_NODE_ID, CHILD_CONFIG)};

struct kscan_composite_config {};

struct kscan_composite_data {
    kscan_callback_t callback;

    const struct device *dev;
};

static int kscan_composite_enable_callback(const struct device *dev) {
    for (int i = 0; i < ARRAY_SIZE(kscan_composite_children); i++) {
        const struct kscan_composite_child_config *cfg = &kscan_composite_children[i];

        const struct device *dev = device_get_binding(cfg->label);
        if (!dev) {
            LOG_WRN("Failed to load child kscan device %s", log_strdup(cfg->label));
            continue;
        }
        kscan_enable_callback(dev);
    }
    return 0;
}

static int kscan_composite_disable_callback(const struct device *dev) {
    for (int i = 0; i < ARRAY_SIZE(kscan_composite_children); i++) {
        const struct kscan_composite_child_config *cfg = &kscan_composite_children[i];

        const struct device *dev = device_get_binding(cfg->label);
        if (!dev) {
            LOG_WRN("Failed to load child kscan device %s", log_strdup(cfg->label));
            continue;
        }
        kscan_disable_callback(dev);
    }
    return 0;
}

static void kscan_composite_child_callback(const struct device *child_dev, uint32_t row,
                                           uint32_t column, bool pressed) {
    // TODO: Ideally we can get this passed into our callback!
    const struct device *dev = device_get_binding(DT_INST_LABEL(0));
    struct kscan_composite_data *data = dev->data;

    for (int i = 0; i < ARRAY_SIZE(kscan_composite_children); i++) {
        const struct kscan_composite_child_config *cfg = &kscan_composite_children[i];

        if (device_get_binding(cfg->label) != child_dev) {
            continue;
        }

        data->callback(dev, row + cfg->row_offset, column + cfg->column_offset, pressed);
    }
}

static int kscan_composite_configure(const struct device *dev, kscan_callback_t callback) {
    struct kscan_composite_data *data = dev->data;

    if (!callback) {
        return -EINVAL;
    }

    for (int i = 0; i < ARRAY_SIZE(kscan_composite_children); i++) {
        const struct kscan_composite_child_config *cfg = &kscan_composite_children[i];

        kscan_config(device_get_binding(cfg->label), &kscan_composite_child_callback);
    }

    data->callback = callback;

    return 0;
}

static int kscan_composite_init(const struct device *dev) {
    struct kscan_composite_data *data = dev->data;

    data->dev = dev;

    return 0;
}

static const struct kscan_driver_api mock_driver_api = {
    .config = kscan_composite_configure,
    .enable_callback = kscan_composite_enable_callback,
    .disable_callback = kscan_composite_disable_callback,
};

static const struct kscan_composite_config kscan_composite_config = {};

static struct kscan_composite_data kscan_composite_data;

DEVICE_DT_INST_DEFINE(0, kscan_composite_init, device_pm_control_nop, &kscan_composite_data,
                      &kscan_composite_config, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &mock_driver_api);
