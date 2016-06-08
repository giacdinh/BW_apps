# BW_apps
This is top level for Apps project.
Any revision should be below. Ex. 1.6.0


Build step
cd 1_6_0
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=/opt/l3mv-linux/2.0/toolchain.cmake ..
make -j8
