//---------------------------------------------------------------------------

// This software is Copyright (c) 2016 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#pragma hdrstop
#include <System.StartUpCopy.hpp>
//---------------------------------------------------------------------------
USEFORM("uGame.cpp", GameForm);
USEFORM("uGameOver.cpp", FrameGameOver); /* TFrame: File Type */
USEFORM("uGamepad.cpp", FrameGamepad); /* TFrame: File Type */
USEFORM("uHighScores.cpp", FrameHighScores); /* TFrame: File Type */
USEFORM("uInstructions.cpp", FrameInstructions); /* TFrame: File Type */
USEFORM("uLevelComplete.cpp", FrameLevelComplete); /* TFrame: File Type */
USEFORM("uMainMenu.cpp", FrameMainMenu); /* TFrame: File Type */
USEFORM("uSettings.cpp", FrameSettings); /* TFrame: File Type */
//---------------------------------------------------------------------------
extern "C" int FMXmain()
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TGameForm), &GameForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
