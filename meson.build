project('FlatTMPFUSE', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

src = [
  'src/main.c',
  'src/fs.c'
]

## BEGIN: DEPS
fuse3_dep = dependency('fuse3',
                      default_options : [
                        'static=true'
                      ])

host_system = host_machine.system()

if host_system == 'linux'
  flattmp_deps = [
    fuse3_dep
  ]  
endif


executable('FlatTMPFUSE', src,
           install : true,
           dependencies : flattmp_deps)
