

# Installation directory prefix.
prefix = '/usr/local'

# Build in debug mode.
enable_debug = True

# Enable libkunquat.
enable_libkunquat = True

# Install development files.
enable_libkunquat_dev = True

# Install Python bindings (requires Python 2.7).
enable_python_bindings = True

# Build and run libkunquat tests.
enable_tests = True

# Build and run long libkunquat tests.
enable_long_tests = False

# Run tests with memory debugging (requires valgrind, disables assert tests).
enable_tests_mem_debug = False

# Enable kunquat-player (requires enable_python_bindings).
enable_player = True

# Enable kunquat-tracker (requires enable_python_bindings and PyQt4).
enable_tracker = True

# Enable kunquat-export (requires libsndfile).
enable_export = True

# Build example Kunquat files.
enable_examples = True


# Select C compiler explicitly
# (supported values: None (autodetect, default), 'gcc', 'clang')
cc = None

# Optimisation level (0..4).
optimise = 4

# Build with libsndfile support (recommended).
with_sndfile = True

# Build WavPack support (recommended).
with_wavpack = True


