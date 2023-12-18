project('fixp', 'cpp', default_options: ['cpp_std=c++23'])

#add_global_arguments('-march=haswell', language: 'cpp')
#add_global_arguments('-fsanitize=address', language: 'cpp')

nanobench = include_directories('third_party/nanobench/src/include/')
sources_bench = [
  'bench.cpp'
]

sciplot = include_directories('third_party/sciplot/')
sources_test = [
  'test.cpp'
]

executable(
  'fixp-test',
  sources: sources_test,
  include_directories: [ sciplot ]
)

executable(
  'fixp-bench',
  sources: sources_bench,
  include_directories: [ nanobench ]
)
