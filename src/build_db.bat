del out\rom.bin.bak
copy out\rom.bin out\rom.bin.bak
del out\rom.bin
%GDK_WIN%\bin\make -f makefile.gen clean release_db
out\rom.bin
pause