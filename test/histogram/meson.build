root_dir = '../../'

histogram_test = executable(
  'histogram_test',
  [
    'main.cpp',
    'histogram.cpp',
    root_dir / 'src/project.cpp',
    root_dir / 'src/simulation.cpp',
    parse_duration_cpp,
    root_dir / 'src/config/agents.cpp',
    root_dir / 'src/config/load.cpp',
    root_dir / 'src/config/tasks.cpp',
    root_dir / 'src/exceptions.cpp',
    root_dir / 'src/histogram.cpp',
  ],
  dependencies : [ test_deps, dependencies ],
  include_directories : ['.', include],
  cpp_args: ['-DUNIT_TEST']
)
test('histogram unit test', histogram_test)
