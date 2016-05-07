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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "setopt.h"

struct setopt_test
{
	int verbose;
	int counter;
	int number;
	const char* file;
	const char* name;
	const char* foo;
};

SETOPT_FLAG(setopt_verbose, struct setopt_test, verbose)

SETOPT_COUNTER(setopt_counter, struct setopt_test, counter)

SETOPT_INTEGER(setopt_number, struct setopt_test, number, strtol, 10)

SETOPT_STRING(setopt_file, struct setopt_test, file)
SETOPT_STRING(setopt_name, struct setopt_test, name)
SETOPT_STRING(setopt_foo, struct setopt_test, foo)

int
main(
	int argc,
	char* argv[]
) {
	(void)argc;
	(void)argv;
	
	/* There's no standard way to reset getopt() for working
	 * on a new command line, so the testing is done in
	 * a single run.
	 */
	
	char* test_argv[] = {
		"test",
		"-v",
		"-i",
		"-i",
		"-f", "/dev/null",
		"-n", "Slartibartfast",
		"-N", "128",
		"bar"
	};
	
	int pass = 0;
	
try_reset:
	
	setopt_named(setopt_verbose, "v", "Be verbose");
	setopt_named(setopt_counter, "i", "Argument counter");
	setopt_named(setopt_number, "N:", "A number of some sort");
	setopt_named(setopt_file, "f:", "File to work with");
	setopt_named(setopt_name, "n:", "Name of something");
	
	setopt_positioned(setopt_foo, "foo", "It's not in the dictionary");
	
	struct setopt_test test;
	
	memset(&test, 0, sizeof(test));
	
#define TEST(Condition) \
	do { \
		if (!(Condition)) { \
			fprintf(stderr, "%s failed", #Condition); \
			abort(); \
		} \
	} while (0) \
	/**/
	
	TEST(setopt(&test, sizeof(test_argv) / sizeof(char*), test_argv) == 0);
	
	fprintf(stdout, "\n");
	
	TEST(test.verbose == 1);
	TEST(test.counter == 2);
	TEST(test.number == 128);
	
	TEST(strcmp(test.file, "/dev/null") == 0);
	TEST(strcmp(test.name, "Slartibartfast") == 0);
	TEST(strcmp(test.foo, "bar") == 0);
	
	fprintf(stdout, "Values looks good, will now print usage:\n\n");
	
	setopt_usage(stdout, "test");
	
	if (!pass++) {
		fprintf(stdout, "Trying a reset, this might fail:\n");
		setopt_try_reset();
		goto try_reset;
	}
	
	fprintf(stdout, "Finally trying to call setopt() without a reset\n\n");
	
	TEST(setopt(&test, sizeof(test_argv) / sizeof(char*), test_argv) == -1);
	TEST(errno == ENOSYS);
	
	return 0;
}

