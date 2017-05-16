

# installation directory prefix
prefix = '/usr/local'

# build in debug mode
enable_debug = True

# enable debug asserts
enable_debug_asserts = False

# enable libkunquat
enable_libkunquat = True

# install libkunquat development files
enable_libkunquat_dev = True

# install Python bindings (requires Python 3.1 or later)
enable_python_bindings = True

# build and run libkunquat tests
enable_tests = True

# build and run long libkunquat tests
enable_long_tests = False

# run tests with memory debugging (requires valgrind, disables assert tests)
enable_tests_mem_debug = False

# enable multithreading (requires with_pthread)
enable_threads = True

# enable kunquat-player (requires Python bindings)
enable_player = True

# enable kunquat-tracker (requires Python bindings, PyQt4, libsndfile and WavPack)
enable_tracker = True

# enable kunquat-export (requires libsndfile)
enable_export = True

# build example Kunquat files
enable_examples = True

# build with POSIX threads
with_pthread = True

# build with libsndfile support
with_sndfile = True

# build with WavPack support
with_wavpack = True


# select C compiler explicitly
# (supported values: None (autodetect, default), 'gcc', 'clang')
cc = None

# optimisation level (0..4)
optimise = 4


