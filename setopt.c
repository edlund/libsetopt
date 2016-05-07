/* libsetopt - A tiny getopt based command line parser library
 * Copyright (C) 2016 Erik Edlund <erik.edlund@32767.se>
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 * 
 *  * Redistributions of source code must retain the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer.
 * 
 *  * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials
 *  provided with the distribution.
 * 
 *  * Neither the name of Erik Edlund, nor the names of its
 *  contributors may be used to endorse or promote products
 *  derived from this software without specific prior written
 *  permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "setopt.h"

#define SETOPT_MIN_CHAR 32
#define SETOPT_MAX_CHAR 126
#define SETOPT_CHARS (SETOPT_MAX_CHAR - SETOPT_MIN_CHAR + 1)

/**
 * A single argument holder.
 */
struct setopt_arg
{
	/** Callback function. */
	setopt_callback callback;
	union {
		/** Specification for this named argument. */
		const char* spec;
		/** Name for this positioned argument. */
		const char* name;
	};
	/** Help line for this argument. */
	const char* help;
};

/**
 * The global state of setopt.
 */
struct setopt_state
{
	/** Determine if setopt() has been called. */
	int called;
	/** Executable name. */
	char* prog;
	/** Specification string passed to getopt(). */
	char spec[SETOPT_CHARS * 2];
	/** Index of where to add the next spec. */
	unsigned int spec_i;
	/** Index of the next positioned argument. */
	unsigned int posd_i;
	/** Slots for storing arg callbacks. */
	struct setopt_arg slots[SETOPT_MIN_CHAR + SETOPT_CHARS];
};

static struct setopt_state state = {
	.called = 0,
	.spec = { '\0' },
	.spec_i = 0,
	.posd_i = 0,
	.slots = { {
		.callback = NULL,
		.spec = NULL,
		.help = NULL
	} }
};

int
setopt_named(
	setopt_callback const callback,
	const char* const spec,
	const char* const help
) {
	if (
		!callback || !spec ||
		!strlen(spec) || strlen(spec) > 2 ||
		spec[0] < SETOPT_MIN_CHAR ||
		spec[0] > SETOPT_MAX_CHAR
	) {
		errno = EINVAL;
		return -1;
	}
	
	strcpy(state.spec + state.spec_i, spec);
	state.spec_i += strlen(spec);
	
	int index = spec[0];
	struct setopt_arg* arg = &state.slots[index];
	arg->callback = callback;
	arg->spec = spec;
	arg->help = help;
	
	return 0;
}

int
setopt_positioned(
	setopt_callback const callback,
	const char* const name,
	const char* const help
) {
	if (!callback || !name) {
		errno = EINVAL;
		return -1;
	}
	if (state.posd_i == SETOPT_MIN_CHAR) {
		errno = ENOMEM;
		return -1;
	}
	
	struct setopt_arg* arg = &state.slots[state.posd_i++];
	arg->callback = callback;
	arg->name = name;
	arg->help = help;
	
	return 0;
}

#define POSITIONED_LOOP(IndexName, ArgName, Code) \
	for ( \
		unsigned int IndexName = 0; \
		IndexName < SETOPT_MIN_CHAR; \
		++IndexName \
	) { \
		struct setopt_arg* ArgName = &state.slots[IndexName]; \
		if (!ArgName->callback) \
			break; \
		Code \
	} \
	/**/

int
setopt_usage(
	FILE* const dest,
	const char* prog
) {
	fprintf(dest, "Usage: %s [%s]", prog, state.spec);
	
	POSITIONED_LOOP(i, arg, {
		fprintf(dest, " %s", arg->name);
	})
	
	fprintf(dest, "\n\n");
	
	for (unsigned int i = 0; i < strlen(state.spec); ++i) {
		if (
			state.spec[i] >= SETOPT_MIN_CHAR &&
			state.spec[i] <= SETOPT_MAX_CHAR &&
			state.spec[i] != ':'
		) {
			struct setopt_arg* arg = &state.slots[(unsigned int)state.spec[i]];
			if (arg->callback)
				fprintf(dest, "\t-%c\t\t%s\n", arg->spec[0], arg->help);
		}
	}
	
	fprintf(dest, "\n");
	
	POSITIONED_LOOP(i, arg, {
		fprintf(dest, "\t%s\t\t%s\n", arg->name, arg->help);
	})
	
	fprintf(dest, "\n");
	
	return 0;
}

int
setopt(
	void* const param,
	int argc,
	char* argv[]
) {
	int option;
	
	if (state.called) {
		/* Multiple calls currently not supported as there is no
		 * standard way to reset getopt().
		 */
		errno = ENOSYS;
		return -1;
	}
	state.called = 1;
	
	while ((option = getopt(argc, argv, state.spec)) != EOF) {
		struct setopt_arg* arg = &state.slots[option];
		if (arg->callback) {
			if (arg->callback(&state, param, optarg) != 0)
				return option;
		} else {
			errno = EINVAL;
			return -1;
		}
	}
	
	POSITIONED_LOOP(i, arg, {
		if (argc - optind == 0) {
			errno = EINVAL;
			return -1;
		}
		if (arg->callback(&state, param, argv[optind++]) != 0) {
			return -1;
		}
	})
	
	return 0;
}

#undef POSITIONED_LOOP

void
setopt_try_reset(
	void
) {
	memset(&state, 0, sizeof(state));
	
	optarg = NULL;
	optind = 0;
	opterr = 0;
	optopt = 0;
}


