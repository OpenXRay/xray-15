mkdir -p ppc && ( cd ppc
gcc -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I..  contourshade.c
gcc -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I..  outimgshade.c
gcc -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I..  outpsshade.c
g++ -arch ppc -flat_namespace -undefined suppress -dynamiclib -o contour.so contourshade.o outimgshade.o outpsshade.o

)

mkdir -p i386 && ( cd i386
gcc -c -O3 -mtune=pentiumpro -fexpensive-optimizations -fforce-mem -finline-functions -funroll-loops -fomit-frame-pointer -frerun-cse-after-loop -fstrength-reduce -fforce-addr -fPIC -ansi -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -DEVIL_ENDIAN -DX86 -DHYPERTHREAD -I..  contourshade.c
gcc -c -O3 -mtune=pentiumpro -fexpensive-optimizations -fforce-mem -finline-functions -funroll-loops -fomit-frame-pointer -frerun-cse-after-loop -fstrength-reduce -fforce-addr -fPIC -ansi -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -DEVIL_ENDIAN -DX86 -DHYPERTHREAD -I..  outimgshade.c
gcc -c -O3 -mtune=pentiumpro -fexpensive-optimizations -fforce-mem -finline-functions -funroll-loops -fomit-frame-pointer -frerun-cse-after-loop -fstrength-reduce -fforce-addr -fPIC -ansi -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -DEVIL_ENDIAN -DX86 -DHYPERTHREAD -I..  outpsshade.c
g++ -flat_namespace -undefined suppress -dynamiclib -o contour.so contourshade.o outimgshade.o outpsshade.o

)

lipo -output contour.so -create ppc/contour.so i386/contour.so
