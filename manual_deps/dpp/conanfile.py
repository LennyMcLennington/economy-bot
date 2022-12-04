from conans import ConanFile, CMake, tools


class DppConan(ConanFile):
    name = "dpp"
    version = "10.0.21"
    license = "Apache-2.0"
    url = "https://github.com/LennyMcLennington/economy-bot"
    description = "C++ Discord API Bot Library"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake_find_package"

    exports = "fix-build.patch"

    requires = "libsodium/1.0.18", "nlohmann_json/3.11.2", "openssl/3.0.7", "opus/1.3.1", "zlib/1.2.13"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        self.run("git clone https://github.com/brainboxdotcc/DPP.git")
        self.run(f"git -C DPP checkout v{self.version}")
        self.run(f"git -C DPP apply ../fix-build.patch")
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_folder="build", source_folder="DPP")
        cmake.build()
        cmake.install()

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package_info(self):
        self.cpp_info.libs = ["dpp"]

