import subprocess, os

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

breathe_projects = {}

if read_the_docs_build:
	subprocess.call('cd ..; wget https://sourceforge.net/projects/plantuml/files/plantuml.jar', shell=True)
	subprocess.call('cd .. ; doxygen', shell=True)
	breathe_projects['knx'] = '../build/xml'
	html_extra_path = ['../build/html']

# -- Project information -----------------------------------------------------

#project = 'knx'
#copyright = '2019, Thomas Kunze'
#author = 'Thomas Kunze'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
#...

#extensions = [ "breathe" ]

#...

# Add any paths that contain templates here, relative to this directory.
#templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
#exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
#html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']

# Breathe Configuration
#breathe_default_project = "knx"