Notice about legality
=====================

This project is still pending permission from the rights holder, Sergey Grigorovich.
As such you should be aware that you could be held responsible for any damages
that are incurred by publishing derivatives based on leaked source code.

OpenGL X-Ray Engine
===================

The goal of this project is to create an OpenGL backend for the X-Ray engine.
By completing this backend we lay the groundwork for making the S.T.A.L.K.E.R. games
multi-platform.

At first we will be working based on Clear Sky after the plugin has been refined and
supports all the features from the DX10 backend we will move on to Call of Pripyat
and port its DX11 backend.

The goal of this fork is purely to make the game multi-platform, without any gameplay
or graphical enhancements.

This repository contains X-Ray Engine sources based on version 1.5.10.
The original engine is used in S.T.A.L.K.E.R. Clear Sky game released by GSC Game World.

Compilation
===========

This project is being developed with Visual Studio 2013.

To build X-Ray Engine you'll need following SDKs:
  * [Windows SDK](http://www.microsoft.com/en-us/download/details.aspx?id=8279) (included in VS2013)
  * [DirectX SDK June 2010](http://www.microsoft.com/en-us/download/details.aspx?id=6812)

Make sure to run `git submodule update --init` after cloning the repository.

Mixed configuration isn't currently configured properly,
Debug/Release should work.

