/*
 * Copyright (C) 2024 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#define _LGPL_SOURCE

#include "lttng-ctl-helper.hpp"

#include <lttng/lttng.h>

lttng_error_code lttng_pause_session(const char *session_name [[maybe_unused]])
{
	if (session_name == nullptr) {
		return LTTNG_ERR_INVALID;
	}

	struct lttcomm_session_msg lsm = {};
	lsm.cmd_type = LTTCOMM_SESSIOND_COMMAND_PAUSE_SESSION;

	const auto copy_ret =
		lttng_strncpy(lsm.session.name, session_name, sizeof(lsm.session.name));
	if (copy_ret) {
		return LTTNG_ERR_INVALID;
	}

	const auto cmd_ret = lttng_ctl_ask_sessiond(&lsm, nullptr);
	if (cmd_ret >= 0) {
		return LTTNG_OK;
	} else {
		return static_cast<lttng_error_code>(-cmd_ret);
	}
}
