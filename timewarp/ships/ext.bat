md %1
cd %1
copy ..\shp%1.dat .
dat -s1 shp%1.dat
dat shp%1.dat -e * -o ./
del shp%1.dat
cd ..