## INSTALL

###  Download

   git clone https://github.com/piScope/Petra-M--DS.git

###  Build

   pip install . --verbos

###  Build step-by-step

   pip install . -C"ext-only=YES"   # build MUMPS
   pip install . -C"swig-only=YES"  # build mumps-solve and SWIG wrapper
   pip install . -C"skip-ext=YES"   # build python extension.


###  Cleaning build files

  python setup.py clean