cd "$(dirname "$0")"
./build.sh
cd ./build_cmake
cp ./libv8_export.* ../../lib/linux_x64/
cd ../
cp ./lib_linux_x64/* ../lib/linux_x64/
rm -rf ./build_cmake
