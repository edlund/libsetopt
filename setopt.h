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

#ifndef SETOPT_H
#define SETOPT_H

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Argument callback.
 */
typedef int (*setopt_callback)(
	void* const state,
	void* const param,
	const char* arg
);

#define SETOPT_CUSTOM(Name, Code) \
	static int \
	Name( \
		void* const state, \
		void* const param, \
		const char* arg \
	) { \
		(void)state; \
		(void)param; \
		(void)arg; \
		Code \
	} \
	/**/

#define SETOPT_FLAG(Name, Type, Member) \
	SETOPT_CUSTOM(Name, { \
		((Type*)param)->Member = 1; \
		return 0; \
	}) \
	/**/

#define SETOPT_COUNTER(Name, Type, Member) \
	SETOPT_CUSTOM(Name, { \
		((Type*)param)->Member += 1; \
		return 0; \
	}) \
	/**/

#define SETOPT_STRING(Name, Type, Member) \
	SETOPT_CUSTOM(Name, { \
		((Type*)param)->Member = arg; \
		return 0; \
	}) \
	/**/

#define SETOPT_INTEGER(Name, Type, Member, ToLongFn, Base) \
	SETOPT_CUSTOM(Name, { \
		char* endptr; \
		errno = 0; \
		((Type*)param)->Member = ToLongFn(arg, &endptr, Base); \
		if (!errno && *endptr != '\0') { \
			errno = EINVAL; \
		} \
		return errno? -1: 0; \
	}) \
	/**/

/**
 * Add a named argument with the callback that should be
 * used when the argument is encountered.
 */
int
setopt_named(
	setopt_callback const callback,
	const char* const spec,
	const char* const help
);

/**
 * Add a positioned argument with the callback that should be
 * used when the argument is encountered - the first call will
 * add the callback for position 1, the second for position 2
 * and so on. 32 positioned arguments are supported.
 */
int
setopt_positioned(
	setopt_callback const callback,
	const char* const name,
	const char* const help
);

/**
 * Print usage instructions to the given file (often stdout
 * or stderr).
 */
int
setopt_usage(
	FILE* const dest,
	const char* prog	
);

/**
 * Dispatch callbacks using the given command line.
 */
int
setopt(
	void* const param,
	int argc,
	char* argv[]
);

/**
 * Try to reset getopt() and setopt() to be reused. Warning;
 * it might not work in all environments.
 */
void
setopt_try_reset(
	void
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

