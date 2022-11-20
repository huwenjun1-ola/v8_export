./build.sh
cd ./build_cmake
cp ./libv8_export.* ../../lib/darwin_arm64/
cd ../
co ./lib_mac_arm64/* ../lib/darwin_arm64/
rm -rf ./build_cmake
