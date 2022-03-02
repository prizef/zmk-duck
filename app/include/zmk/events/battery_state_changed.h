/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr.h>
#include <zmk/event_manager.h>

struct zmk_battery_state_changed {
    // TODO: Other battery channels
    uint8_t state_of_charge;
};

ZMK_EVENT_DECLARE(zmk_battery_state_changed);