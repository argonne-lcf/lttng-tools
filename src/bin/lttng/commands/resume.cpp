/*
 * Copyright (C) 2024 Erica Bugden <ebugden@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#include "../command.hpp"
#include "common/argpar/argpar.hpp"
#include "common/format.hpp"
#include "lttng/lttng.h"

#include <vendor/optional.hpp>

#include <string>

namespace {
enum { OPT_HELP };

const struct argpar_opt_descr resume_options[] = {
	{ OPT_HELP, 'h', "help", false },
	ARGPAR_OPT_DESCR_SENTINEL,
};

void print_command_help()
{
	fmt::print("Usage: lttng [GENERAL OPTIONS] resume [SESSION]\n");
}

std::string get_default_session_name()
{
	const auto session_name =
		lttng::make_unique_wrapper<char, lttng::memory::free>(get_session_name());

	if (session_name == nullptr) {
		return "";
	} else {
		return std::string(session_name.get());
	}
}

cmd_error_code resume_tracing_session(std::string session_name)
{
	const lttng_error_code resume_ret_code = lttng_resume_session(session_name.c_str());

	if (resume_ret_code != LTTNG_OK) {
		switch (resume_ret_code) {
		case LTTNG_ERR_NO_SESSIOND:
			ERR_FMT("Cannot handle resume request. {}.",
				lttng_strerror(LTTNG_ERR_NO_SESSIOND));
			return CMD_ERROR;

		case LTTNG_ERR_SESS_NOT_FOUND:
			ERR_FMT("Session '{}' does not exist.", session_name);
			return CMD_ERROR;

		case LTTNG_ERR_SESSION_DATA_CONSUMPTION_ALREADY_ONGOING:
			WARN_FMT(
				"Session '{}': {}.",
				session_name,
				lttng_strerror(LTTNG_ERR_SESSION_DATA_CONSUMPTION_ALREADY_ONGOING));
			return CMD_WARNING;

		default:
			ERR_FMT("Session '{}': Error while attempting to resume data consumption.",
				session_name);
			return CMD_ERROR;
		}
	}

	fmt::print("Session '{}': Resumed userspace data consumption.\n", session_name);

	return CMD_SUCCESS;
}
} /* namespace */

/*
 *  cmd_resume
 *
 *  The `resume [SESSION]` command.
 *
 *  Resuming a recording session restarts the consumption of tracing
 *  data to the disk or network. The opposite command, lttng-pause stops
 *  the consumption of trace data.
 *
 *  Since this command is a prototype:
 *
 *  • Pausing and resuming consumption of kernel trace data is
 *    unsupported. Currently, only userspace is supported. If the
 *    session is also tracing kernel space then consumption of that data
 *    will not be paused.
 *
 *  • Not all general options are implemented (e.g. machine interface)
 *    we just handle them without failing.
 */
int cmd_resume(int argc, const char **argv)
{
	/* Skip "resume" in the list of arguments */
	const int my_argc = argc - 1;
	const char **my_argv = argv + 1;

	/* Handle machine interface option */
	if (lttng_opt_mi) {
		ERR_FMT("Machine interface not supported for this command.");
		return CMD_UNSUPPORTED;
	}

	/* Parse command arguments */
	argpar::Iter<nonstd::optional<argpar::Item>> argpar_iter(my_argc, my_argv, resume_options);
	std::string session_name;

	while (true) {
		nonstd::optional<argpar::Item> argpar_item;

		try {
			argpar_item = argpar_iter.next();
		} catch (argpar::UnknownOptError& unknown_option_error) {
			ERR_FMT("Unrecognized command option: {}", unknown_option_error.name());
			print_command_help();
			return CMD_ERROR;
		}

		if (!argpar_item) {
			/* No more arguments to parse. */
			break;
		}

		if (argpar_item->isOpt()) {
			/* If the argument is a command option */
			const argpar::OptItemView option = argpar_item->asOpt();

			switch (option.descr().id) {
			case OPT_HELP:
				print_command_help();
				return CMD_SUCCESS;
			}
		} else {
			/* If the argument is a session name */
			const argpar::NonOptItemView session_name_item = argpar_item->asNonOpt();
			session_name = std::string(session_name_item.arg());
		}
	}

	if (session_name.empty()) {
		session_name = get_default_session_name();

		if (session_name.empty()) {
			ERR_FMT("No tracing session to resume was defined and there "
				"is no default session in `.lttngrc`.");
			return CMD_ERROR;
		}
	}

	return resume_tracing_session(session_name);
}
