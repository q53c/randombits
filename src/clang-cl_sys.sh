"/usr/lib/llvm-19/bin/clang" -cc1 -triple x86_64-pc-windows-msvc19.33.0 -emit-obj -mincremental-linker-compatible -dumpdir a- -disable-free -clear-ast-before-backend -disable-llvm-verifier -discard-value-names -main-file-name sample3.c -mrelocation-model pic -pic-level 2 -mframe-pointer=none -relaxed-aliasing -fmath-errno -ffp-contract=on -fno-rounding-math -mconstructor-aliases -fms-volatile -funwind-tables=2 -target-cpu x86-64 -mllvm -x86-asm-syntax=intel -tune-cpu generic -D_MT -flto-visibility-public-std -stack-protector 2 -fno-rtti -fdiagnostics-format msvc -fms-kernel -cfguard -fdebug-compilation-dir=/home/u0_a274/Working/xbltexe/cmdline -v -fcoverage-compilation-dir=/home/u0_a274/Working/xbltexe/cmdline -resource-dir /usr/lib/llvm-19/lib/clang/19 -D WINNT=1 -D _AMD64_=1 -internal-isystem /usr/lib/llvm-19/lib/clang/19/include -internal-isystem $HOME/winsdk/sdk/Include/10.0.26100/shared -internal-isystem $HOME/winsdk/sdk/Include/10.0.26100/km -internal-isystem $HOME/winsdk/sdk/Include/10.0.26100/km/crt -ferror-limit 19 -fno-use-cxa-atexit -fms-extensions -fms-compatibility -fms-compatibility-version=19.33 -fskip-odr-check-in-gmf -fdelayed-template-parsing -fcolor-diagnostics --vfsoverlay vfs_map.yaml -faddrsig -o sample3_ref.obj -x c sample3.c

"/usr/lib/llvm-19/bin/lld-link" -out:ref.exe -nologo \
        -libpath:$HOME/winsdk/sdk/Lib/10.0.26100/km/x64 \
        ntoskrnl.lib cfg_support_v1.lib bufferoverflowfastfailk.lib hal.lib \
        /SUBSYSTEM:NATIVE /ENTRY:DriverEntry \
        -guard:cf /DEBUG:FULL \
        /OPT:REF /OPT:ICF /MERGE:"_TEXT=.text;_PAGE=PAGE" /NODEFAULTLIB /SECTION:INIT,d /VERSION:10.0 \
        /TSAWARE:NO \
        /vfsoverlay:vfs_map.yaml sample3_ref.obj