//---------------------------------------------------------------------------

// This software is Copyright (c) 2016 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "AudioManager.h"



/* TAudioManager */


#include <System.hpp>

bool GLoaded = false;

#if defined(_PLAT_IOS) || defined(_PLAT_MACOS)
void __fastcall oncompleteionIosProc( NSInteger SystemSndID, void*& AData )
{
 //  place here the code to run when a sound finish playing
}
#endif

__fastcall TAudioManager::TAudioManager( )
{
  try
  {
	fSoundsList = new TList();
	#if defined(_PLAT_ANDROID)
	_di_JObject obj = SharedActivity()->getSystemService(TJContext::JavaClass->AUDIO_SERVICE);
	fAudioMgr = TJAudioManager::Wrap((_di_IJavaInstance)obj);
	fSoundPool = TJSoundPool::JavaClass->init( 3, TJAudioManager::JavaClass->STREAM_MUSIC, 0 );
	#endif
  }
  catch( Exception & E )
  {
    throw Exception( "[TAudioManager.Create] : " + E.Message );
  }
}


__fastcall TAudioManager::~TAudioManager( )
{
  int i = 0;
  PSoundRec wRec = NULL;
  try
  {
    for ( int stop = 0, i = fSoundsList->Count - 1; i >= stop; i--)
    {
	  wRec = (PSoundRec)fSoundsList->Items[i];
      delete wRec;
      fSoundsList->Delete( i );
	}
	delete fSoundsList;
	fSoundsList = NULL;
	#if defined (_PLAT_ANDROID)
	  fSoundPool = NULL;
	  fAudioMgr = NULL;
	#endif
	// todo check:  inherited::Destroy();
  }
  catch( Exception & E )
  {
	throw Exception( "[TAudioManager.Destroy] : " + E.Message );
  }
}


int __fastcall TAudioManager::AddSound( String ASoundFile )
{
  int result = 0;
  PSoundRec wSndRec = NULL;
  #if defined (_PLAT_ANDROID)
  JSoundPool_OnLoadCompleteListener *wOnAndroidSndComplete;
  #endif
  #if defined (_PLAT_IOS)
  NSInteger wSndID = 0;
  CFStringRef wNSFilename;
  CFURLRef wNSURL;
  CFRunLoopRef wCFRunLoopRef;
  CFStringRef winRunLoopMode;
  #endif
  try
  {
	result = - 1;
	wSndRec = new TSoundRec;
	wSndRec->SFilename = ASoundFile;
	wSndRec->SNameExt = ExtractFileName( ASoundFile );
	wSndRec->SName = ChangeFileExt( wSndRec->SNameExt, "" );

	#if defined (_PLAT_ANDROID)
	GLoaded = false;
	wSndRec->SID = fSoundPool->load( StringToJString( ASoundFile ), 1 );
	while ( ! GLoaded )
	{
	  int soundID = fSoundPool->play( wSndRec->SID, 0, 0, 0, 0, 0 );
	  if (soundID>0) {
		fSoundPool->stop( wSndRec->SID );
		GLoaded = true;
	  }
	  Sleep( 10 );
	  Application->ProcessMessages();
	}
	#endif
	#if defined(_PLAT_IOS)
	wNSFilename = CFStringCreateWithCharacters( NULL, (const UniChar *)ASoundFile.c_str(), ASoundFile.Length( ) );
	wNSURL = CFURLCreateWithFileSystemPath( NULL, wNSFilename, kCFURLPOSIXPathStyle, false );
	AudioServicesCreateSystemSoundID( wNSURL, &wSndID );
	wSndRec->SID = wSndID;
	AudioServicesAddSystemSoundCompletion( wSndID, NULL, NULL, oncompleteionIosProc, NULL );
	CFRelease(wNSFilename);
	#endif
	result = fSoundsList->Add( wSndRec );
  }
  catch( Exception & E )
  {
	throw Exception( "[TAudioManager.AddSound] : " + E.Message );
  }
  return result;
}


void __fastcall TAudioManager::DeleteSound( int Aindex )
{
  PSoundRec wRec = NULL;
  try
  {
	if ( Aindex < fSoundsList->Count )
	{
	  wRec = (PSoundRec)fSoundsList->Items[Aindex];
	  #if defined(_PLAT_ANDROID)
	  fSoundPool->unload( wRec->SID );
	  #endif
	  #if defined(_PLAT_IOS)
	  AudioServicesDisposeSystemSoundID( wRec->SID );
	  #endif
	  delete wRec;
	  fSoundsList->Delete( Aindex );
	}
  }
  catch( Exception & E )
  {
	throw Exception( "[TAudioManager.DeleteSound] : " + E.Message );
  }
}


void __fastcall TAudioManager::DeleteSound( String AName )
{
  int i = 0;
  try
  {
    for ( int stop = fSoundsList->Count - 1, i = 0; i <= stop; i++)
    {
	  if ( CompareText( ((PSoundRec)fSoundsList->Items[i] )->SName, AName ) == 0 )
      {
		DeleteSound( i );
        break;
      }
	}
  }
  catch( Exception & E )
  {
	throw Exception( "[TAudioManager.PlaySound] : " + E.Message );
  }
}


void __fastcall TAudioManager::PlaySound( int Aindex )
{
  PSoundRec wRec = NULL;
  #if defined (_PLAT_ANDROID)
  double wCurrVolume = 0.0, wMaxVolume = 0.0;
  double wVolume = 0.0;
  #endif

  try
  {
	if ( Aindex < fSoundsList->Count )
	{
	  wRec = (PSoundRec)fSoundsList->Items[Aindex];
	  #if defined(_PLAT_ANDROID)
	  if (fAudioMgr)
	  {
		wCurrVolume = fAudioMgr->getStreamVolume( TJAudioManager::JavaClass->STREAM_MUSIC );
		wMaxVolume = fAudioMgr->getStreamMaxVolume( TJAudioManager::JavaClass->STREAM_MUSIC );
		wVolume = wCurrVolume / wMaxVolume;
		fSoundPool->play( wRec->SID, wVolume, wVolume, 1, 0, 1 );
	  }
	  #endif
	  #if defined(_PLAT_IOS)
	  AudioServicesAddSystemSoundCompletion( wRec->SID, NULL, NULL, oncompleteionIosProc, NULL );
	  AudioServicesPlaySystemSound( wRec->SID );
	  #else
	   #ifdef _PLAT_MACOS
		TNSSound::Wrap(TNSSound::Alloc()->initWithContentsOfFile(StrToNSStr(wRec->SFilename),true))->play();
	   #endif
	  #endif
	  #if defined (_PLAT_MSWINDOWS)
	  sndPlaySound(wRec->SFilename.w_str(), SND_NODEFAULT | SND_ASYNC);
	  #endif
	}
  }
  catch( Exception & E )
  {
	throw Exception( "[Unknown Name] : " + E.Message );
  }
}


void __fastcall TAudioManager::PlaySound( String AName )
{
  int i = 0;
  try
  {
	for ( int stop = fSoundsList->Count - 1, i = 0; i <= stop; i++)
	{
	  if ( CompareText( ((PSoundRec)fSoundsList->Items[i] )->SName, AName ) == 0 )
      {
        PlaySound( i );
		break;
      }
    }
  }
  catch( Exception & E )
  {
	throw Exception( "[TAudioManager.PlaySound] : " + E.Message );
  }
}


int __fastcall TAudioManager::GetSoundsCount( )
{
  return fSoundsList->Count;
}


PSoundRec __fastcall TAudioManager::GetSoundFromIndex( int Aindex )
{
  if ( Aindex < fSoundsList->Count )
	return (PSoundRec)fSoundsList->Items[Aindex];
  else
	return NULL;
}

