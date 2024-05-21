/*
 * Copyright (C) 2024 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#define _LGPL_SOURCE
#include "resume.hpp"

lttng_error_code cmd_resume_session(const ltt_session::locked_ref &session)
{
	return LTTNG_ERR_NOT_SUPPORTED;
}