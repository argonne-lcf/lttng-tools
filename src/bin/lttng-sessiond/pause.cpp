/*
 * Copyright (C) 2024 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#define _LGPL_SOURCE
#include "pause.hpp"

#include <common/exception.hpp>
#include <common/format.hpp>

void cmd_pause_session(const ltt_session::locked_ref& session)
{
	try {
		session->pause();
	} catch (const lttng::sessiond::exceptions::session_data_consumption_already_paused& ex) {
		std::throw_with_nested(lttng::ctl::error(
				fmt::format("Failed to pause session: session_name={}, uid={}", session->name, session->uid),
				LTTNG_ERR_SESSION_DATA_CONSUMPTION_ALREADY_PAUSED,
				LTTNG_SOURCE_LOCATION()));
	} catch (const lttng::runtime_error& ex) {
		std::throw_with_nested(lttng::ctl::error(
				fmt::format("Failed to pause session: session_name={}, uid={}", session->name, session->uid),
				LTTNG_ERR_UNK,
				LTTNG_SOURCE_LOCATION()));
	}
}