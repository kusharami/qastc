TEMPLATE = subdirs
SUBDIRS = \
    plugin \
    qastc_tests

qastc_tests.depends = plugin 