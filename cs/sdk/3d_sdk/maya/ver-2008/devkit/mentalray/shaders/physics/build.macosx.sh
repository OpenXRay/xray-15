g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  dgsutil.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  dgsshade.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  dielecshade.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  partishade.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  pathshade.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  physlight.cpp
g++ -c -O3 -arch ppc -fPIC -dynamic -fno-common -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DMACOSX -D_REENTRANT -I.  physlens.cpp
g++ -arch ppc -flat_namespace -undefined suppress -dynamiclib -o physics.so dgsutil.o dgsshade.o dielecshade.o partishade.o pathshade.o physlight.o physlens.o

