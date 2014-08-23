========================================================================
                    CUSTOM APPWIZARD: SDKAPWZ
========================================================================


AppWizard has created a SDKAPWZ DLL for you.  This DLL is the starting point
for writing your custom AppWizard.  It demonstrates the basics of creating a
custom AppWizard.

Although your custom AppWizard is a DLL, it has the special suffix AWX.  When
you build SDKAPWZ.awx, it will automatically be copied to your Template
directory.  Your custom AppWizard will then appear as choice in the Project
Type drop-list in the New Workspace and Insert Project dialogs.  To run your
custom AppWizard, simply create a new workspace (or insert a project into the
current workspace), and select your custom AppWizard from the Project Type
drop-list.

This file contains a summary of what you will find in each of the files that
make up your SDKAPWZ DLL.


SDKAPWZ.mak
    This project file is compatible with the Visual C++ development
    environment.  It is also compatible with the NMAKE program provided with
    Visual C++.

    To build a debug version of the program from the MS-DOS prompt, type
    nmake /f SDKAPWZ.mak CFG="Win32 Debug".

    To build a release version of the program, type
    nmake /f SDKAPWZ.mak CFG="Win32 Release".

SDKAPWZ.cpp
    This file is the main DLL source file that contains the definition of
    DllMain().  It also exports the function GetCustomAppWizClass(), which
    returns a pointer to the one instance of this custom AppWizard's
    CCustomAppWiz-derived class.

SDKAPWZ.h
    This file is the main header file for the DLL.  It includes your 
    RESOURCE.H file.

SDKAPWZ.rc
    This file is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes all of your custom AppWizard's templates as
    custom resources of type "TEMPLATE".  These resources are pointers to the
    files in your project's TEMPLATE directory.  This file can be directly
    edited in the Visual C++ development environment.  However, you will
    probably want to edit your templates by opening the template files directly
    in the source editor rather than by editing the "TEMPLATE" resources from
    the Visual C++ resource editor.

SDKAPWZ.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////
Custom AppWizard Interface:

SDKAPWZAw.h, SDKAPWZAw.cpp - the CCustomAppWiz class
    These files contain your CCustomAppWiz-derived class,
    CSDKAPWZAppWiz.  This class contains virtual member functions which
    MFCAPWZ.DLL calls to initialize your custom AppWizard and to query which
    step to pop up at a given time.  This class also contains m_Dictionary,
    a CMapStringToString member variable, which maps template macro names
    to their values.

/////////////////////////////////////////////////////////////////////////////
Dialogs:

Chooser.h, Chooser.cpp - the dialog chooser
    These files contain your CDialogChooser class.  The class maintains
    pointers to each of your steps, keeps track of which step is currently
    up, and handles calls to your custom AppWizard class's member functions
    Next(...) and Back(...).

Cstm*Dlg.h, Cstm*Dlg.cpp - the dialog classes
    These files contain the dialog classes for all of your custom AppWizard's
    new steps.  They derive from CAppWizStepDlg and override
    CAppWizStepDlg::OnDismiss.

/////////////////////////////////////////////////////////////////////////////
Help Support:

MakeHelp.bat
    Use this batch file to create your custom AppWizard's Help file,
    SDKAPWZ.hlp.

SDKAPWZ.hpj
    This file is the Help Project file used by the Help compiler to create
    your custom AppWizard's Help file.

HLP\SDKAPWZ.rtf
    This file contains an empty topic for each new step you generated.
    You may fill out the topics using any rich-text-format
    editor such as Microsoft Word.

/////////////////////////////////////////////////////////////////////////////
Template Files:

Template\
    Put your template files in this directory.  Template files are stored
    in your custom AppWizard as custom resources of type "TEMPLATE", and are
    used by your custom AppWizard to determine the contents of the files it
    generates. When you add a new template file to this directory, you must
    import that file as a "TEMPLATE" custom resource into SDKAPWZ.rc.  Be
    sure to select the "External File" checkbox on the custom resource's
    property page.

Template\Confirm.inf
    In this template you should put a description of the project your
    custom AppWizard generates.  The file uses template macros to customize the
    text to reflect which options were selected by the custom AppWizard user.
    When the custom AppWizard user clicks the "Finish" button, MFCAPWZ.DLL
    parses this template and sends the output to the New Project Information
    dialog.

Template\NewProj.inf
    This template lists all of the templates other than Confirm.inf and
    NewProj.inf which your custom AppWizard will use to generate a project.
    After MFCAPWZ.DLL parses this template, the output lists the other
    templates to be parsed and what the output files should be called.
        See the documentation on custom AppWizards for a more complete
    description of this template and the project generation process in general.

/////////////////////////////////////////////////////////////////////////////
Other Standard Files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SDKAPWZ.pch and a precompiled types file named StdAfx.obj.

Rresource.h
    This is the standard header file, which defines new resource IDs.
    Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other Notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.
