
rem Use as: extract.bat shpalabo
rem that will extract shpalabo.dat to shpalabo/


mkdir %1
dat.exe -e -o %1/ %1.dat *
