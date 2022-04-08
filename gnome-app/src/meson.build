semi_lab_sources = [
  'main.c',
  'semi_lab-window.c',
    'semi_lab-application.c',
  ]

semi_lab_deps = [
  dependency('gtk4'),
]

gnome = import('gnome')

semi_lab_sources += gnome.compile_resources('semi_lab-resources',
  'semi_lab.gresource.xml',
  c_name: 'semi_lab'
)

executable('semi-lab', semi_lab_sources,
  dependencies: semi_lab_deps,
  install: true,
)
