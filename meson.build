project('fixp', 'cpp', default_options: ['cpp_std=c++23'])

#add_global_arguments('-march=haswell', language: 'cpp')

nanobench = include_directories('thirdparty/nanobench/src/include/')
sources_bench = [
  'bench.cpp'
]

sources_test = [
  'test.cpp'
]

executable(
  'fixp-test',
  sources: sources_test,
)

executable(
  'fixp-bench',
  sources: sources_bench,
  include_directories: [nanobench]
)
