project('command-handler', 'cpp', license: 'GPL-3.0', default_options : ['cpp_std=c++20'])
fs = import('fs')

include_dir = include_directories('include')
libfmt = dependency('fmt')
libdpp = dependency('dpp')

subdir('src')

library(meson.project_name(), main_sources, dependencies: [libfmt, libdpp], include_directories: include_dir)
