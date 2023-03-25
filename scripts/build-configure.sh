pushd "$(dirname "$0")"/../build/cmake/

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja -Wno-dev .

popd 