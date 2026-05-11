#
# make
# make all      -- build the app (debug)
#
# make release  -- build optimised binary
#
# make run      -- build and launch the app
#
# make test     -- run C++ unit tests via CMake/ctest
#
# make install  -- install binary to /usr/local/bin
#
# make clean    -- remove Swift and CMake build artefacts
#
.PHONY: all release run test install clean

all:
	swift build

release:
	swift build -c release

run:
	swift run Kalah

test: cmake-build
	$(MAKE) -Ccmake-build unit_tests
	ctest --test-dir cmake-build --output-on-failure

install: release
	install -m 755 .build/release/Kalah /usr/local/bin/Kalah

clean:
	rm -rf .build cmake-build

cmake-build:
	mkdir $@
	cmake -B$@ -DCMAKE_BUILD_TYPE=RelWithDebInfo
