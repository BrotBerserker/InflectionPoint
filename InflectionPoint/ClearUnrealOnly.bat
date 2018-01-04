:repeat
rmdir Intermediate /S /Q 
if EXIST Intermediate goto repeat
rmdir Saved /S /Q
if EXIST Saved goto repeat
rmdir Binaries /S /Q
if EXIST Binaries goto repeat