project('economy-bot', 'cpp', license: 'GPL-3.0', default_options : ['cpp_std=c++20'])
fs = import('fs')

libfmt = dependency('fmt')
libdpp = dependency('dpp')
libmongocxx = dependency('mongocxx', modules: ['mongo::mongocxx_shared'])
libbsoncxx = dependency('mongocxx', modules: ['mongo::bsoncxx_shared'])

subdir('src')

executable(meson.project_name(), main_sources, dependencies: [libfmt, libdpp, libmongocxx, libbsoncxx])
