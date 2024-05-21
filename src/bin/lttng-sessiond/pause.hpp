/*
 * Copyright (C) 2024 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#ifndef LTTNG_SESSIOND_PAUSE_H
#define LTTNG_SESSIOND_PAUSE_H

#include "session.hpp"

#include <common/string-utils/c-string-view.hpp>

#include <lttng/lttng-error.h>

lttng_error_code cmd_pause_session(const ltt_session::locked_ref& session);

#endif /* LTTNG_SESSIOND_PAUSE_H */
