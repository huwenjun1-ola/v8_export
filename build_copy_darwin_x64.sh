./build.sh
cd ./build_cmake
cp ./libv8_export.* ../../lib/darwin_x64/
cd "$(dirname "$0")"
co ./lib_mac_x64/* ../../lib/darwin_x64/
rm -rf ./build_cmake