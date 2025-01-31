declare -a arr=(
    lib/gl 
    lib/driver
    lib/glx
    lib/stubs
    lib/utils
    lib/wgl
    unittest/cpp
)

## now loop through the above array
for i in "${arr[@]}"
do
   echo "format $i"
   find $i/* -iname '*.hpp' -o -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
done

#find lib/gl/* -iname '*.hpp' -o -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
