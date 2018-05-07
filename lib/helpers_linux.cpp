// =============================================================================
/*!
 *
 *   This file is part of the ATRACSYS fusionTrack library.
 *   Copyright (C) 2003-2015 by Atracsys LLC. All rights reserved.
 *
 *   \file helpers_linux.cpp
 *   \brief Helping functions used by sample applications
 *
 */
// =============================================================================

#include "helpers.hpp"

#include <ftkInterface.h>
#include <ftkTypes.h>

#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/timeb.h>


// -----------------------------------------------------------------------------
// Linux - Mutex mechanism wrapping
// -----------------------------------------------------------------------------

class LinMutex : public Mutex
{
protected:
    pthread_mutex_t mMutex;
public:
    LinMutex()
    {
        pthread_mutexattr_t mutexAttr;
        ( void ) pthread_mutexattr_init( &mutexAttr );
        ( void ) pthread_mutexattr_settype( &mutexAttr,
                                            PTHREAD_MUTEX_RECURSIVE_NP );
        ( void ) pthread_mutex_init( &mMutex, &mutexAttr );
        ( void ) pthread_mutexattr_destroy( &mutexAttr );
    }

    ~LinMutex()
    {
        ( void ) pthread_mutex_destroy( &mMutex );
    }

    // Return true if mutex lock is successful
    bool lock( uint32 timeout = 0xFFFFFFFF )
    {
        ( void ) pthread_mutex_lock( &mMutex ); return true;
    }

    // Release a previously locked mutex
    void release()
    {
        ( void ) pthread_mutex_unlock( &mMutex );
    }
};

// ----------------------------------------------------------------------------

Mutex* createMutex()
{
    return new LinMutex();
}

int detectKeyboardHit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    oldf = fcntl( STDIN_FILENO, F_GETFL, 0 );
    fcntl( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK );

    ch = getchar();

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    fcntl( STDIN_FILENO, F_SETFL, oldf );

    if ( ch != EOF )
    {
        ungetc( ch, stdin );
        return 1;
    }

    return 0;
}

void waitForKeyboardHit( unsigned timeout )
{
    while ( detectKeyboardHit() == 0 )
    {
        usleep( timeout );
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


long getMilliCount()
{
    timeb tb;
    ftime( &tb );
    return tb.millitm + ( tb.time & 0xfffff ) * 1000;
}

void sleep( long uTimeMS )
{
    usleep( uTimeMS * 1000 );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class LinThread : public Thread
{
public:
    LinThread( OnProcess onProcess = 0 );
    virtual ~LinThread();

    bool start( void* userData = 0 );

    bool stop();

    bool isStarted() const;

    bool isTerminated();
protected:
    pthread_t* _Handle;
    void* _UserData;

    enum Status
    {
        THREAD_CREATED,
        THREAD_STARTED,
        THREAD_TERMINATED
    };

    Status _Status;

    static void* threadProc( void* arg );
};

LinThread::LinThread( OnProcess onProcess )
    : Thread( onProcess )
    , _Handle( 0 )
    , _UserData( 0 )
    , _Status( THREAD_CREATED )
{}

LinThread::~LinThread()
{
    stop();
}

bool LinThread::start( void* userData )
{
    if ( _Handle != 0 )
    {
        error( "Thread already started" );
        return false;
    }
    if ( mOnProcess == 0 )
    {
        error( "Thread without process function" );
        return false;
    }
    _UserData = userData;

    // Suggestion to use _beginthreadex...
    _Handle = new pthread_t();

    if ( _Handle == 0 )
    {
        return false;
    }

    pthread_attr_t threadAttr;

    // init thread attributes structure
    int retval( pthread_attr_init( &threadAttr ) );
    if ( retval != 0 )
    {
        return false;
    }

    retval = pthread_attr_setdetachstate( &threadAttr,
                                          PTHREAD_CREATE_JOINABLE );
    if ( retval != 0 )
    {
        return false;
    }

    retval = pthread_attr_setinheritsched( &threadAttr,
                                           PTHREAD_INHERIT_SCHED );
    if ( retval != 0 )
    {
        return false;
    }

    // create thread
    retval = pthread_create( _Handle, &threadAttr,
                             LinThread::threadProc,
                             reinterpret_cast< void* >( this ) );
    if ( retval != 0 )
    {
        return false;
    }

    // free attribute stucture
    retval = pthread_attr_destroy( &threadAttr );
    if ( retval != 0 )
    {
        return false;
    }

    return true;
}

bool LinThread::isStarted() const
{
    return _Handle != 0 && _Status == THREAD_STARTED;
}

bool LinThread::isTerminated()
{
    return _Handle != 0 && _Status == THREAD_TERMINATED;
}

bool LinThread::stop()
{
    if ( _Handle == 0 )
    {
        return true;
    }
    if ( _Status == THREAD_CREATED || _Status == THREAD_TERMINATED )
    {
        return true;
    }

    int retval( pthread_join( *_Handle, 0 ) );
    if ( retval != 0 )
    {
        return false;
    }

    return true;
}

void* LinThread::threadProc( void* arg )
{
    if ( arg == 0 )
    {
        return 0;
    }
    LinThread* thread = reinterpret_cast< LinThread* >( arg );
    thread->_Status = LinThread::THREAD_STARTED;


    thread->mOnProcess( *thread, thread->_UserData );

    thread->_Status = LinThread::THREAD_TERMINATED;

    return 0;
}

Thread* createThread( OnProcess onProcess )
{
    return new LinThread( onProcess );
}
