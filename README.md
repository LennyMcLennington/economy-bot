# Economy Bot

The name is a placeholder for now, until we come up with a good name. Also, there's nothing to put here currently because the bot doesn't have any features yet.

# Dependencies

Required to have already installed on the system: A C++20 compiler, python3, meson, conan, git, pkgconf

For the rest of the dependencies: run `./setup_dependencies.py` to install them with conan. Pkg-config files will be written to the `conan_deps` directory.

# Building

Run `meson setup build -Dpkg_config_path="$PWD/conan_deps"` to configure the build system.

Then use `ninja -C build` to build the software.
