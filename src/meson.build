gnome_semilab_sources = [
  'main.c',
  'gnome-semilab-workspace.c',
  'gnome-semilab-application.c',
  'gnome-semilab-window.c',
  'gsp-create-project-widget.c',
  'gnome-semilab-global.c',
  'utils.c',
  'csv_reader.c',
  'sqlimit.c',
  'consts.c',
]

gnome_semilab_marshal = gnome.genmarshal('gnome-semilab-marshal',
            internal: true,
              prefix: 'gnome_semilab_marshal',
             sources: ['gnome-semilab-marshal.defs'],
  valist_marshallers: true,
)

gnome_semilab_sources += gnome_semilab_marshal

# graph_sources = []
# graph_include_directories = []
# graph_link_with = []
# graph_deps = [
#   dependency('libplot'),
# ]

# subdir('graph')

# graph = static_library('graph', graph_sources,
#            dependencies: graph_deps,
# 	           c_args: release_args,
#     include_directories: graph_include_directories,
#               link_with: graph_link_with,
#   gnu_symbol_visibility: 'hidden',
# )

# gnome_semilab_static = [
#   graph,
# ]

# The path in Flatpak sandbox
libcsv_inc = include_directories('/usr/include')
# https://github.com/mesonbuild/meson/issues/6235
# Prebuilt objects are specified with the objects keyword argument.
libcsv_lib = static_library('csv', objects :'/app/lib/libcsv.a')
libcsv_dep = declare_dependency(link_with : libcsv_lib, include_directories : libcsv_inc)

gnome_semilab_deps = [
  libgtk_dep,
  libadwaita_dep,
  libcsv_dep,
  dependency('xlsxwriter'),
  dependency('gsl'),
  dependency('plplot'),
]

gnome_semilab_sources += gnome.compile_resources('gnome-semilab-resources',
  'gnome-semilab.gresource.xml',
  c_name: 'gnome_semilab',
)

executable('gnome-semilab', gnome_semilab_sources,
  dependencies: gnome_semilab_deps,
       install: true,
  # link_whole: gnome_semilab_static,
)
