mkdir build
pushd build
cmake -A Win32 ..
msbuild /t:Build /p:Configuration=Release lua-qoi.sln
popd