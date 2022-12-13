gnome_semilab_sources = [
  'main.c',
  'gnome-semilab-application.c',
  'gnome-semilab-window.c',
]

gnome_semilab_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1'),
]

gnome_semilab_sources += gnome.compile_resources('gnome-semilab-resources',
  'gnome-semilab.gresource.xml',
  c_name: 'gnome_semilab'
)

executable('gnome-semilab', gnome_semilab_sources,
  dependencies: gnome_semilab_deps,
       install: true,
)
