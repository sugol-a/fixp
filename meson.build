project('fixp', 'cpp', default_options: ['cpp_std=c++23'])

add_global_arguments('-march=haswell', language: 'cpp')

include = include_directories('thirdparty/nanobench/src/include/')
sources = [
  'test.cpp'
]

executable(
  'fixp-test',
  sources: sources,
  include_directories: include
)
