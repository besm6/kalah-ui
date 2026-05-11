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
# make test     -- run unit tests
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

test:
	swift test

bundle: release
	rm -rf $(APP)
	mkdir -p $(BUNDLE)/MacOS $(BUNDLE)/Resources
	cp app/Info.plist $(BUNDLE)/Info.plist
	cp .build/release/Kalah $(BUNDLE)/MacOS/kalah

install: bundle
	cp -a $(APP) $$HOME/Applications/

clean:
	rm -rf .build $(APP)
