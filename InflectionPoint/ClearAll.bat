:repeat
del *.VC.db
if EXIST *.VC.db goto repeat
del *.VC.VC.db
if EXIST *.VC.VC.db goto repeat
del *.sln
if EXIST *.sln goto repeat
rmdir Intermediate /S /Q 
if EXIST Intermediate goto repeat
rmdir Saved /S /Q
if EXIST Saved goto repeat
rmdir Binaries /S /Q
if EXIST Binaries goto repeat
rmdir .vs /S /Q
if EXIST .vs goto repeat