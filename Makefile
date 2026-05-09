#
# make
# make all      -- build everything
#
# make test     -- run unit tests
#
# make install  -- install binaries to /usr/local
#
# make clean    -- remove build files
#
.PHONY: all install clean test debug bundle

all:    build
	$(MAKE) -Cbuild $@

test:   build
	$(MAKE) -Cbuild unit_tests controller_tests
	ctest --test-dir build --output-on-failure

install: build
	$(MAKE) -Cbuild $@

clean:
	rm -rf build

build:
	mkdir $@
	cmake -B$@ -DCMAKE_BUILD_TYPE=RelWithDebInfo

debug:
	mkdir build
	cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug

bundle: all
	rm -rf Kalah.app
	mkdir -p Kalah.app/Contents/MacOS Kalah.app/Contents/libs
	cp macos/Info.plist Kalah.app/Contents/Info.plist
	cp build/bin/kalah Kalah.app/Contents/MacOS/kalah
	dylibbundler -od -b \
	    -x Kalah.app/Contents/MacOS/kalah \
	    -d Kalah.app/Contents/libs/ \
	    -p @executable_path/../libs/
