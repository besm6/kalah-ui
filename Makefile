#
# make
# make all      -- build the app (debug)
#
# make release  -- build optimised binary
#
# make run      -- build and launch the app
#
# make bundle   -- create Kalah.app bundle
#
# make test     -- run C++ unit tests via CMake/ctest
#
# make install  -- install binary to /usr/local/bin
#
# make clean    -- remove Swift and CMake build artefacts
#
.PHONY: all release run bundle test install clean

APP    = Kalah.app
BUNDLE = $(APP)/Contents

all:
	swift build

release:
	swift build -c release

run:
	swift run Kalah

test: cmake-build
	$(MAKE) -Ccmake-build unit_tests
	ctest --test-dir cmake-build --output-on-failure
	swift test

bundle: release
	rm -rf $(APP)
	mkdir -p $(BUNDLE)/MacOS $(BUNDLE)/Resources
	cp macos/Info.plist $(BUNDLE)/Info.plist
	cp .build/release/Kalah $(BUNDLE)/MacOS/kalah

install: release
	install -m 755 .build/release/Kalah /usr/local/bin/Kalah

clean:
	rm -rf .build cmake-build $(APP)

cmake-build:
	mkdir $@
	cmake -B$@ -DCMAKE_BUILD_TYPE=RelWithDebInfo
