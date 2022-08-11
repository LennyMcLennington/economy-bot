project('economy-bot', 'cpp', license: 'GPL-3.0', default_options : ['cpp_std=c++20', 'warning_level=2'])
fs = import('fs')

libfmt = dependency('fmt')
libdpp = dependency('dpp')
libmongocxx = dependency('libmongocxx', modules: ['mongo::mongocxx_shared'])
libbsoncxx = dependency('libmongocxx', modules: ['mongo::bsoncxx_shared'])

subdir('src')

executable(meson.project_name(), main_sources, dependencies: [libfmt, libdpp, libmongocxx, libbsoncxx])
