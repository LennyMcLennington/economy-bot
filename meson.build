project('economy-bot', 'cpp', license: 'AGPL-3.0-only', default_options : ['cpp_std=c++20', 'warning_level=2'])
fs = import('fs')


libfmt = dependency('fmt')
libdpp = dependency('dpp')
libmongocxx = dependency('libmongocxx-static')
libbsoncxx = dependency('libbsoncxx-static')

subdir('src')

executable(meson.project_name(), main_sources, dependencies: [libfmt, libdpp, libmongocxx, libbsoncxx])
