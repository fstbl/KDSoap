TEMPLATE = subdirs
SUBDIRS  = src kdwsdl2cpp features
unittests: SUBDIRS += testtools unittests
unittests: SUBDIRS += examples
CONFIG   += ordered
MAJOR_VERSION = 1 ### extract from $$VERSION

unix:DEFAULT_INSTALL_PREFIX = /usr/local/KDAB/KDSoap-$$VERSION
win32:DEFAULT_INSTALL_PREFIX = "C:\KDAB\KDSoap"-$$VERSION

# for backw. compat. we still allow manual invocation of qmake using PREFIX:
isEmpty( KDSOAP_INSTALL_PREFIX ): KDSOAP_INSTALL_PREFIX=$$PREFIX

# if still empty we use the default:
isEmpty( KDSOAP_INSTALL_PREFIX ): KDSOAP_INSTALL_PREFIX=$$DEFAULT_INSTALL_PREFIX

# if the default was either set by configure or set by the line above:
equals( KDSOAP_INSTALL_PREFIX, $$DEFAULT_INSTALL_PREFIX ){
    INSTALL_PREFIX=$$DEFAULT_INSTALL_PREFIX
    unix:message( "No install prefix given, using default of" $$DEFAULT_INSTALL_PREFIX (use configure.sh -prefix DIR to specify))
    !unix:message( "No install prefix given, using default of" $$DEFAULT_INSTALL_PREFIX (use configure -prefix DIR to specify))
} else {
    INSTALL_PREFIX=\"$$KDSOAP_INSTALL_PREFIX\"
}

DEBUG_SUFFIX=""
VERSION_SUFFIX=""
CONFIG(debug, debug|release) {
  !unix: DEBUG_SUFFIX = d
}
!unix:!mac:!staticlib:VERSION_SUFFIX=$$MAJOR_VERSION

KDSOAPLIB = kdsoap$$DEBUG_SUFFIX$$VERSION_SUFFIX
KDSOAPSERVERLIB = kdsoap-server$$DEBUG_SUFFIX$$VERSION_SUFFIX


message(Install prefix is $$INSTALL_PREFIX)
message(This is KD Soap version $$VERSION)

# These files are in the build directory (Because "somecommand >> somefile" puts them there)
CONFQMAKE_CACHE = "$${OUT_PWD}/.confqmake.cache"
QMAKE_CACHE = "$${OUT_PWD}/.qmake.cache"

# Create a .qmake.cache file
unix:MESSAGE = '\\'$$LITERAL_HASH\\' KDAB qmake cache file: following lines autogenerated during qmake run'
!unix:MESSAGE = $$LITERAL_HASH' KDAB qmake cache file: following lines autogenerated during qmake run'

system('echo $${MESSAGE} > $${QMAKE_CACHE}')

TMP_SOURCE_DIR = $${IN_PWD}
TMP_BUILD_DIR = $${OUT_PWD}
system('echo TOP_SOURCE_DIR=$${TMP_SOURCE_DIR} >> $${QMAKE_CACHE}')
system('echo TOP_BUILD_DIR=$${TMP_BUILD_DIR} >> $${QMAKE_CACHE}')
windows:INSTALL_PREFIX=$$replace(INSTALL_PREFIX, \\\\, /)
system('echo INSTALL_PREFIX=$$INSTALL_PREFIX >> $${QMAKE_CACHE}')
system('echo KDSOAPLIB=$$KDSOAPLIB >> $${QMAKE_CACHE}')
system('echo KDSOAPSERVERLIB=$$KDSOAPSERVERLIB >> $${QMAKE_CACHE}')

# forward make test calls to unittests:
test.target=test
unittests {
unix:!macx:test.commands=export LD_LIBRARY_PATH=\"$${OUT_PWD}/lib\":$$(LD_LIBRARY_PATH); (cd unittests && make test)
macx:test.commands=export DYLD_LIBRARY_PATH=\"$${OUT_PWD}/lib\":$$(DYLD_LIBRARY_PATH); (cd unittests && make test)
win32:test.commands=(cd unittests && $(MAKE) test)
}
test.depends = first
QMAKE_EXTRA_TARGETS += test

# install licenses: 
licenses.files = LICENSE.GPL.txt LICENSE.US.txt LICENSE.txt
licenses.path = $$INSTALL_PREFIX
INSTALLS += licenses

readme.files = README.txt
readme.path = $$INSTALL_PREFIX
INSTALLS += readme

prifiles.files = kdsoap.pri kdwsdl2cpp.pri
prifiles.path = $$INSTALL_PREFIX
INSTALLS += prifiles

OTHER_FILES += configure.sh configure.bat kdsoap.pri kdwsdl2cpp.pri doc/CHANGES*
