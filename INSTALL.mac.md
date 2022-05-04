# OS X compilation

##### Requirements:
* Xcode 12.4
* Qt 5.12.11

##### Preparation:
1. Follow the instructions collected in the [macOS](README.mac.md) to build the required tools / third-party dependencies and get the source code of Desktop repository.
2. Set the env variables `MEGAQTPATH` to a valid Qt installation path and `VCPKGPATH` to a directory containing a valid vcpkg installation.
2. Run the script `installer_mac.sh` to build the project and generate the application bundle for Desktop application. You can generate full packages using either cmake or qmake. Check script options with `--help` flag. Build directory is `Release_x64`
3. Enjoy!

