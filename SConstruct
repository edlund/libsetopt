
env = Environment(
	CCFLAGS=" ".join([
		"-Wall",
		"-Wextra",
		"-pedantic",
		"-std=c11",
		"-D_GNU_SOURCE",
		"-D_FILE_OFFSET_BITS=64"
	])
)

env.Append(LIBPATH="#/")

env.StaticLibrary("setopt", "setopt.c")
env.Program("test", "test.c", LIBS=[
	"setopt"
])

