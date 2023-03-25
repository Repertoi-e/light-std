SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm -rf $SCRIPT_DIR/../build

$SCRIPT_DIR/../third-party/bin/premake/premake5 cmake

pushd "$(dirname "$0")"/../build/cmake/

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja -Wno-dev .
cmake --build .

popd 