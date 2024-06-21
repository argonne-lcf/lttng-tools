/*
 * Copyright (C) 2024 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#ifndef LTTNG_SESSIOND_RESUME_H
#define LTTNG_SESSIOND_RESUME_H

#include "session.hpp"

#include <lttng/lttng-error.h>

void cmd_resume_session(const ltt_session::locked_ref& session);

#endif /* LTTNG_SESSIOND_RESUME_H */
