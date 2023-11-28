# How to clone and build LSCSIM
$ git clone --recurse-submodules https://github.com/hyunzmama/LSCSIM.git

$ cd LSCSIM

$ . .initialize

$ cd ..; mkdir build; cd build

$ ccmake ../LSCSIM

$ make -j10
