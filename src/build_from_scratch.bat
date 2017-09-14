del out\* /s /f /q
del out\*.* /s /f /q
%GDK_WIN%\bin\make -f makefile.gen clean release
out\rom.bin
pause