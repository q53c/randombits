# llvm-msvc 生成 /MD 库

why? 插件编写如果链接的是 `/MT`, 会导致 `opt.exe` 加载插件后，内部有多份 `llvm`，这种情况下卸载插件时会crash。

llvm-msvc 官方给出的编译指令，由于打开了 `RPMALLOC`，项目会强行编成 `/MT`

```
if(LLVM_ENABLE_RPMALLOC)
  # ...
  # Override the C runtime allocator with the in-tree rpmalloc
  set(LLVM_INTEGRATED_CRT_ALLOC "${CMAKE_CURRENT_SOURCE_DIR}/lib/Support")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
```

修改后的configure命令

```
cmake .. -G "Visual Studio 17 2022" -A X64 -DLLVM_ENABLE_RPMALLOC=OFF -DCMAKE_CXX_FLAGS="/utf-8" -DCMAKE_C_FLAGS="/utf-8" -DLLVM_ENABLE_PACK_PDB=ON -DLLDB_ENABLE_PYTHON=OFF -DLLVM_ENABLE_PROJECTS="clang;lld" -DCMAKE_INSTALL_PREFIX=E:\llvm\rdeb-md -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_TARGETS_TO_BUILD="X86;AArch64" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_USE_CRT_RELEASE=MD -DLLVM_USE_CRT_RELWITHDEBINFO=MD -DLLVM_USE_CRT_MINSIZEREL=MD -DLLVM_USE_CRT_DEBUG=MDd -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DLLVM_ENABLE_ML=ON ../llvm
```

构建

```
msbuild /m -p:Configuration=RelWithDebInfo INSTALL.vcxproj
```

