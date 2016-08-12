#ifndef AudioManagerH
#define AudioManagerH


#include <System.Classes.hpp>
#include <System.SysUtils.hpp>
#include <System.Types.hpp>
#include <System.UITypes.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Forms.hpp>
#if defined(_PLAT_ANDROID)
#include <androidapi.jni.media.hpp>
#include <FMX.Helpers.Android.hpp>
#include <androidapi.jni.JavaTypes.hpp>
#include <Androidapi.JNI.GraphicsContentViewText.hpp>
#include <Androidapi.JNIBridge.hpp>
#include <androidapi.helpers.hpp>
#endif
#if defined(_PLAT_IOS)
#include <MacApi.CoreFoundation.hpp>
#include <FMX.Platform.iOS.hpp>
#include <iOSapi.CocoaTypes.hpp>
#include <iOSapi.AVFoundation.hpp>
#include <iOSapi.Foundation.hpp>
#else
#if defined(_PLAT_MACOS)
#include <MacApi.CoreFoundation.hpp>
#include <FMX.Platform.Mac.hpp>
#include <Macapi.CocoaTypes.hpp>
#include <Macapi.AppKit.hpp>
#include <Macapi.Foundation.hpp>
#include <Macapi.Helpers.hpp>
#endif
#endif
#if defined(_PLAT_MSWINDOWS)
#include <Winapi.MMSystem.hpp>
#endif

struct TSoundRec {
  String SFilename;
  String SName;
  String SNameExt;
  int SID;
};

typedef TSoundRec *PSoundRec;

class TAudioManager: public System::TObject {
  private:
  TList *fSoundsList;
  #ifdef _PLAT_ANDROID
  _di_JAudioManager fAudioMgr;
  _di_JSoundPool fSoundPool;
  #endif
  int __fastcall GetSoundsCount( );
  PSoundRec __fastcall GetSoundFromIndex( int Aindex );
  public:
  __fastcall TAudioManager( );
  __fastcall virtual ~TAudioManager( );
  int __fastcall AddSound( String ASoundFile );
  void __fastcall DeleteSound( String AName );
  void __fastcall DeleteSound( int Aindex );
  void __fastcall PlaySound( String AName );
  void __fastcall PlaySound( int Aindex );
  __property int SoundsCount = { read = GetSoundsCount };
  __property PSoundRec Sounds [ int Aindex ] = { read = GetSoundFromIndex };
};


extern bool GLoaded;
#if defined(_PLAT_IOS)
const char _libAudioToolbox[] = "/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox";
extern "C" void AudioServicesPlaySystemSound( NSInteger inSystemSoundID );
extern "C" void AudioServicesCreateSystemSoundID( CFURLRef inFileURL, PNSInteger SystemSoundID );
extern "C" void AudioServicesDisposeSystemSoundID( NSInteger inSystemSoundID );
extern "C" void AudioServicesAddSystemSoundCompletion( NSInteger inSystemSoundID, CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode, void* inCompletionRoutine, CFURLRef inClientData );
#endif

#endif //  AudioManagerH