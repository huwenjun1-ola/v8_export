cd "$(dirname "$0")"
cmake --build ./cmake-build-release-visual-studio --target v8_export
cd "$(dirname "$0")"
cp ./lib_win_x64/* ../lib/win_x64/