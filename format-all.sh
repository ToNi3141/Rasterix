declare -a files=(
    lib/gl 
    lib/driver
    lib/glx
    lib/stubs
    lib/utils
    lib/wgl
    unittest/cpp
)

for i in "${files[@]}"
do
    echo "format $i"
    find $i/* -iname '*.hpp' -o -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' | xargs clang-format -i
done

