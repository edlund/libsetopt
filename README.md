
# libsetopt - A tiny getopt based command line parser library

Origin: https://bitbucket.org/erikedlund/libsetopt

Mirror: https://github.com/edlund/libsetopt

*Warning:* Since setopt is based on getopt(), it's not thread
safe or reentrant. The API originally used a setopt state
managed by the user, however, in order to avoid giving the
wrong signals to developers, it's now using a single global
state to look every bit as thread unsafe and non-reentrant as
it is.

Compared to using only getopt(), setopt() can be a little
harder to initially understand as it works through callbacks
most often created through utility macros. Hopefully the
resulting code will be more readable and more modular once
one get used to it.

The basic idea is that setopt() will set values stored in a
container (structure) of some sort based on options passed
on the command line. Later, the values stored are validated,
but it's possible to write custom callbacks that validates
arguments before they're stored.

