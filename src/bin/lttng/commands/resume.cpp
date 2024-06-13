/*
 * Copyright (C) 2024 Erica Bugden <ebugden@efficios.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#include "../command.hpp"
#include "common/format.hpp"
#include "common/argpar-utils/argpar-utils.hpp"
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
	int res;

	/* Handle machine interface option */
	if (lttng_opt_mi) {
		ERR_FMT("Machine interface not supported for this command.");
		return CMD_UNSUPPORTED;
	}

	/* Parse command arguments */
	struct argpar_iter *argpar_iter = NULL;
	const struct argpar_item *argpar_item = NULL;
	std::string session_name;

	argpar_iter = argpar_iter_create(my_argc, my_argv, resume_options);
	if (!argpar_iter) {
		ERR("Failed to allocate an argpar iter.");
		res = CMD_ERROR;
		goto end;
	}

	while (true) {
		enum parse_next_item_status status;

		status = parse_next_item(argpar_iter,
					 &argpar_item,
					 1,
					 my_argv,
					 false,
					 nullptr,
					 "While parsing resume action:");
		if (status == PARSE_NEXT_ITEM_STATUS_ERROR) {
			res = CMD_ERROR;
			goto end;
		} else if (status == PARSE_NEXT_ITEM_STATUS_END) {
			break;
		}

		LTTNG_ASSERT(status == PARSE_NEXT_ITEM_STATUS_OK);

		if (argpar_item_type(argpar_item) == ARGPAR_ITEM_TYPE_OPT) {
			/* If the argument is a command option */
			const struct argpar_opt_descr *descr =
				argpar_item_opt_descr(argpar_item);

			switch (descr->id) {
			case OPT_HELP:
				print_command_help();
				res = CMD_SUCCESS;
				goto end;
			}
		} else {
			/* If the argument is a session name */
			const char *arg = argpar_item_non_opt_arg(argpar_item);
			session_name = std::string(arg);
		}
	}

	if (session_name.empty()) {
		session_name = get_default_session_name();

		if (session_name.empty()) {
			ERR_FMT("No tracing session to resume was defined and there "
				"is no default session in `.lttngrc`.");
			res = CMD_ERROR;
			goto end;
		}
	}

	res = resume_tracing_session(session_name);

end:
	argpar_item_destroy(argpar_item);
	argpar_iter_destroy(argpar_iter);
	return res;
}
