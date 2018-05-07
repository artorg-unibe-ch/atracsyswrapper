// ============================================================================
/*!
 *
 *   This file is part of the ATRACSYS fusionTrack library.
 *   Copyright (C) 2003-2015 by Atracsys LLC. All rights reserved.
 *
 *   \file helpers_windows.cpp
 *   \brief Helping functions used by sample applications
 *
 */
// ============================================================================

#include "helpers.hpp"

#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <stdio.h>

using namespace std;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

bool isLaunchedFromExplorer()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE), &csbi))
    {
        printf( "GetConsoleScreenBufferInfo failed: %lu\n", GetLastError());
        return FALSE;
    }

    // if cursor position is (0,0) then we were launched in a separate console
    return ((!csbi.dwCursorPosition.X) && (!csbi.dwCursorPosition.Y));
}

// Get millisecond counter
//long getMilliCount()
//{
//    enum State
//    {
//        INIT_PERF_COUNTER,
//        USE_PERF_COUNTER,
//        INIT_TIME_GET_TIME,
//        USE_TIME_GET_TIME,
//        USE_TICK_COUNT
//    };
//
//    static State state = INIT_PERF_COUNTER;
//    static float sfMult = -1.0;
//
//    while ( 1 )
//    {
//        switch ( state )
//        {
//
//            case INIT_PERF_COUNTER:
//            {
//                LARGE_INTEGER largeInteger;
//                if ( QueryPerformanceFrequency( &largeInteger ) )
//                {
//                    sfMult = 1000.0f / ( float ) largeInteger.QuadPart;
//                    state = USE_PERF_COUNTER;
//                }
//                else
//                {
//                    state = INIT_TIME_GET_TIME;
//                    break;
//                }
//            }
//
//            case USE_PERF_COUNTER:
//            {
//                LARGE_INTEGER largeInteger;
//
//                if ( QueryPerformanceCounter( &largeInteger ) )
//                {
//                    return ( unsigned long ) ( sfMult *
//                                               ( float ) largeInteger.QuadPart );
//                }
//                else
//                {
//                    state = INIT_TIME_GET_TIME;
//                    break;
//                }
//            }
//
//            case INIT_TIME_GET_TIME:
//            {
//                if ( timeBeginPeriod( 1 ) != TIMERR_NOERROR )
//                {
//                    state = USE_TICK_COUNT;
//                    break;
//                }
//                state = USE_TIME_GET_TIME;
//            }
//
//            case USE_TIME_GET_TIME:
//            {
//                return timeGetTime();
//            }
//
//            case USE_TICK_COUNT:
//            {
//                return GetTickCount();
//            }
//        }
//    }
//}

// ----------------------------------------------------------------------------

void sleep( long ms )
{
    Sleep( ms );
}

#ifdef ATR_BORLAND
#define _kbhit kbhit
#endif

// Wait for a keyboard hit
void waitForKeyboardHit()
{
    while ( ! _kbhit() )
    {
        Sleep( 100 );
    }
    _getch();
}

#ifdef ATR_BORLAND
#undef_kbhit
#endif

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Multithreading helpers

class WinMutex : public Mutex
{
public:
    WinMutex();

    virtual ~WinMutex();

    // Return true if mutex lock is successful
    bool lock( uint32 timeout = 0xFFFFFFFF );

    // Release a previously locked mutex
    void release();
protected:
    HANDLE mMutex;
};

WinMutex::WinMutex()
        : Mutex()
        , mMutex( CreateMutex( 0, false, 0 ) )
{}

WinMutex::~WinMutex()
{
    CloseHandle( mMutex ); mMutex = 0;
}

bool WinMutex::lock( uint32 timeout )
{
    return mMutex != 0 &&
           WaitForSingleObject( mMutex, timeout ) == WAIT_OBJECT_0;
}

void WinMutex::release()
{
    if ( mMutex != 0 )
    {
        ReleaseMutex( mMutex );
    }
}

// ----------------------------------------------------------------------------

Mutex* createMutex()
{
    return ( Mutex* ) new WinMutex();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


class WinThread : public Thread
{

public:
    WinThread( OnProcess onProcess = 0 );
    virtual ~WinThread();

    bool start( void* userData = 0 );

    bool isStarted() const;
    bool isTerminated();

    bool stop();
protected:
    HANDLE mHandle;
    void* mUserData;

    enum Status
    {
        THREAD_CREATED,
        THREAD_STARTED,
        THREAD_TERMINATED
    };

    Status mStatus;

    enum ErrorCodes : DWORD
    {
        THREAD_INVALID_PARAMETER,
        THREAD_INVALID_PROCESS_FN,
        THREAD_ERROR = 0,
        THREAD_OK = 1,
    };

    static DWORD WINAPI threadProc( LPVOID lpParameter );
};

WinThread::WinThread( OnProcess onProcess )
        : Thread( onProcess )
        , mHandle( 0 )
        , mUserData( 0 )
        , mStatus( THREAD_CREATED )
{}

WinThread::~WinThread()
{
    stop();
}

bool WinThread::start( void* userData )
{
    if ( mHandle != 0 )
    {
        error( "Thread already started" );
        return false;
    }
    if ( mOnProcess == 0 )
    {
        error( "Thread without process function" );
        return false;
    }
    mUserData = userData;

    // Suggestion to use _beginthreadex...
    mHandle = CreateThread( 0, 0, threadProc,
                            reinterpret_cast< void* >( this ), 0, 0 );
    return mHandle != 0 ? true : false;
}

bool WinThread::isStarted() const
{
    return mHandle != 0 && mStatus == THREAD_STARTED;
}

bool WinThread::isTerminated()
{
    return mHandle != 0 && mStatus == THREAD_TERMINATED;
}

bool WinThread::stop()
{
    if ( mHandle == 0 )
    {
        return true; // Silent error
    }
    // DWORD exitCode;
    if ( ! isTerminated() )
    {
        if ( ! TerminateThread( mHandle, 0L ) )
        {
            cerr << "Cannot kill thread" << endl;
        }
    }

    CloseHandle( mHandle );

    return true;
}

DWORD WINAPI WinThread::threadProc( LPVOID lpParameter )
{
    if ( ! lpParameter )
    {
        return THREAD_INVALID_PARAMETER;
    }
    WinThread* wt = reinterpret_cast< WinThread* >( lpParameter );
    wt->mStatus = THREAD_STARTED;
    if ( ! wt->mOnProcess )
    {
        return THREAD_INVALID_PROCESS_FN;
    }

    bool ret = wt->mOnProcess( *wt, wt->mUserData );

    wt->mStatus = THREAD_TERMINATED;
    return ret ? THREAD_OK : THREAD_ERROR;
}

Thread* createThread( OnProcess onProcess )
{
    return new WinThread( onProcess );
}

