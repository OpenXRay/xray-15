//***************************************************************************
//* SceneAPI - Implementation of Scene Extension API for 3D Studio MAX 1.2
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* November 2, 1996	CCJ Initial coding
//* January  8, 1997	CCJ Added material editor slot access
//* March   15, 1997	CCJ Added scene materials access
//* November 10, 1997	CCJ Ported to MAX Release 2.0
//*
//* This class implements a couple of missing API calls.
//* 
//* WARNING:
//* These functions depend on the internal structure of 3D Studio MAX 2.0.
//* Do not attempt to use it with other versions.
//*

#ifndef __CJAPIEXT__H
#define __CJAPIEXT__H


class SceneAPI {
public:
	SceneAPI(Interface* i);

	MtlBase* GetMtlSlot(int i);
	MtlBaseLib* GetSceneMtls();

private:
	void FindScene();

	Interface* ip;
	ReferenceMaker* scene;
};

#endif	// __CJAPIEXT__H