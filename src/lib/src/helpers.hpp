// ============================================================================
/*!
*
*   This file is part of the ATRACSYS fusionTrack library.
*   Copyright (C) 2003-2015 by Atracsys LLC. All rights reserved.
*
*   \file helpers.hpp
*   \brief Helping functions used by sample applications
*
*/
// ============================================================================
#pragma once

#include <ftkInterface.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef ATR_OSX
#ifndef PTHREAD_MUTEX_RECURSIVE_NP
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif
#endif

/** \addtogroup Platform dependent functions
* \{
*/

/** \brief Get millisecond counter.
*
* This function gets a counter with a (roughly) milliseconds granularity.
*
* \return a counter of milliseconds from an undefined reference.
*/
//long getMilliCount();

/** \brief is the software having its own console?
*
* This function is used to know if the program was launch from an explorer.
* It needs to be called in main() before printing to stdout
* See http://support.microsoft.com/kb/99115
*
* \return true if program is in its own console (cursor at 0,0) or
*         false if it was launched from an existing console
*/
bool isLaunchedFromExplorer();

/** \brief Function waiting for a keyboard hit.
*
* This function allows to detect a hit on (almost) any key. Known non-detected
* keys are:
*  - shift;
*  - Caps lock;
*  - Ctrl;
*  - Alt;
*  - Alt Gr.
* The function blocks and only returns when a `valid' key is stroke.
*/
void waitForKeyboardHit();

/** \brief Function pausing the current execution process / thread.
*
* This function stops the current execution thread / process for at least
* the given amount of time.
*
* \param[in] ms amount of time to wait, in milliseconds.
*/
void sleep( long ms );


/**
* \}
*/

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/** \brief Function displaying error messages on the standard error output.
*
* This function displays an error message and exits the current process. The
* value returned by the program is 1. On windows, the user is asked to press
* a key to exit.
*
* \param[in] message error message to print.
* \param[in] dontWaitForKeyboard, setting to true to avoid being prompted before exiting program
*/
inline void error( const char* message, bool dontWaitForKeyboard = false )
{
    std::cerr << "ERROR: " << message << std::endl;

#ifdef ATR_WIN
    if ( ! dontWaitForKeyboard )
    {
        std::cout << "Press the \"ANY\" key to quit" << std::endl;
        waitForKeyboardHit();
    }
#endif

    exit( 1 );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/** \brief Helper struct holding a device serial number and its type.
*/
struct DeviceData
{
    uint64 SerialNumber;
    ftkDeviceType Type;
};

/** \brief Callback function for devices.
*
* This function assigns the serial number to the \c user argument, so that the
* device serial number is retrieved.
*
* \param[in] sn serial number of the discovered device.
* \param[out] user pointer on a DeviceData instance where the information will
* be copied.
*/
inline void deviceEnumerator( uint64 sn, void* user, ftkDeviceType type )
{
    if ( user != 0 )
    {
        DeviceData* ptr = reinterpret_cast< DeviceData* >( user );
        ptr->SerialNumber = sn;
        ptr->Type = type;
    }
}

/** \brief Callback function for fusionTrack devices.
*
* This function assigns the serial number to the \c user argument, so that the
* device serial number is retrieved. If the enumerated device is a simulator,
* the device is considered as not detected.
*
* \param[in] sn serial number of the discovered device.
* \param[out] user pointer on the location where the serial will be copied.
* \param[in] devType type of the device.
*/
inline void fusionTrackEnumerator( uint64 sn, void* user,
                                   ftkDeviceType devType )
{
    if ( user != 0 )
    {
        if ( devType != DEV_SIMULATOR )
        {
            DeviceData* ptr = reinterpret_cast< DeviceData* >( user );
            ptr->SerialNumber = sn;
            ptr->Type = devType;
        }
        else
        {
            std::cerr
                    << "ERROR: This sample cannot be used with the simulator"
                    << std::endl;
            DeviceData* ptr = reinterpret_cast< DeviceData* >( user );
            ptr->SerialNumber = sn;
            ptr->Type = DEV_UNKNOWN_DEVICE;
        }
    }
}

/** \brief Function exiting the program after displaying the error.
*
* This function retrieves the last error and displays the corresponding
* string in the terminal.
*
* The typical usage is:
* \code
* ftkError err( ... );
* if ( err != FTK_OK )
* {
*     checkError( lib );
* }
* \endcode
*
* \param[in] lib library handle.
*/
inline void checkError( ftkLibrary lib, bool dontWaitForKeyboard = false,  bool quit = true )
{
    char message[ 1024u ];
    ftkError err( ftkGetLastErrorString( lib, 1024u, message ) );
    if ( err == FTK_OK )
    {
        std::cerr << message << std::endl;
    }
    else
    {
        std::cerr << "Uninitialised library handle provided" << std::endl;
    }

    if ( quit )
    {
#ifdef ATR_WIN
        if ( ! dontWaitForKeyboard )
        {
            std::cout << "Press the \"ANY\" key to exit" << std::endl;
            waitForKeyboardHit();
        }
#endif
        exit( 1 );
    }
}

/** \brief Function enumerating the devices and keeping the last one.
*
* This function uses the ftkEnumerateDevices library function and the
* deviceEnumerator callback so that the last discovered device is kept.
*
* If no device is discovered, the execution is stopped by a call to error();
*
* \param[in] lib initialised library handle.
* \param[in] allowSimulator setting to \c false requires to discover only real
* \param[in] quiet setting to \c true to disactivate printouts
* \param[in] dontWaitForKeyboard, setting to true to avoid being prompted before exiting program
*
* devices (i.e. the simulator device is not retrieved).
*
* \return the serial number of the discovered device.
*/
inline DeviceData retrieveLastDevice( ftkLibrary lib, bool allowSimulator = true, bool quiet = false, bool dontWaitForKeyboard = false )
{
    DeviceData device;
    device.SerialNumber = 0uLL;
    // Scan for devices
    ftkError err( FTK_OK );
    if ( allowSimulator )
    {
        err = ftkEnumerateDevices( lib, deviceEnumerator, &device );
    }
    else
    {
        err = ftkEnumerateDevices( lib, fusionTrackEnumerator, &device );
    }

    if ( err > FTK_OK )
    {
        checkError( lib, dontWaitForKeyboard );
    }
    else if ( err < FTK_OK )
    {
        if ( ! quiet )
            checkError( lib, dontWaitForKeyboard, false );
    }

    if ( device.SerialNumber == 0uLL )
    {
        error( "No device connected", dontWaitForKeyboard );
    }
    std::string text;
    switch ( device.Type )
    {
        case DEV_SPRYTRACK_180:
            text = "sTk 180";
            break;
        case DEV_FUSIONTRACK_500:
            text = "fTk 500";
            break;
        case DEV_FUSIONTRACK_250:
            text = "fTk 250";
            break;
        case DEV_SIMULATOR:
            text = "fTk simulator";
            break;
        default:
            text = " UNKNOWN";
            error( "Unknown type", dontWaitForKeyboard );
    }
    if ( ! quiet )
    {
        std::cout << "Detected one";

        std::cout << " with serial number 0x" << std::setw( 16u )
                  << std::setfill( '0' ) << std::hex << device.SerialNumber << std::dec
                  << std::endl << std::setfill( '\0' );
    }

    return device;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/** \brief Helper class reading the ftkError string.
*
* This class provides an API to interpret the error string gotten from
* ftkGetLastErrorString.
*
* \code
* char tmp[ 1024u ];
* if ( ftkGetLastErrorString( lib, 1024u, tmp ) == FTK_OK )
* {
*     ErrorReader reader;
*     if ( ! reader.parse( tmp ) )
*     {
*         cerr << "Cannot interpret received error:" << endl << tmp << endl;
*     }
*     else
*     {
*         if ( reader.hasError( FTK_ERR_INTERNAL ) )
*         {
*             cout << "Internal error" << endl;
*         }
*         // ...
*     }
* }
* \endcode
*/
class ErrorReader
{
public:
    /** \brief Default constructor.
    */
    ErrorReader();
    /** \brief Destructor, does nothing fancy.
    */
    ~ErrorReader();
    /** \brief Parsing method.
    *
    * This method parses the error string. It is not a XML parser, as the
    * error syntax is very simple. It extracts the errors, warnings and other
    * messages from the provided string.
    *
    * Syntax errors are reported on std::cerr.
    *
    * \param[in] str string to be parsed.
    *
    * \retval true if the parsing could be done successfully,
    * \retval false if an error occurred,
    */
    bool parseErrorString( const std::string& str );
    /** \brief Getter for a given error.
    *
    * This method checks whether is the given error was flagged.
    *
    * \param[in] err error code to be checked.
    *
    * \retval true if the given error was flagged in the XML string,
    * \retval false if \c err is a warning, if no errors were flagged or if
    * \c err is not found in the flagged errors.
    */
    bool hasError( ftkError err ) const;
    /** \brief Getter for a given warning.
    *
    * This method checks whether is the given warning was flagged.
    *
    * \param[in] err warning code to be checked.
    *
    * \retval true if the given warning was flagged in the XML string,
    * \retval false if \c war is an error, if no warning were flagged or if
    * \c war is not found in the flagged errors.
    */
    bool hasWarning( ftkError war ) const;
    /** \brief Getter for OK status.
    *
    * This method checks if no errors or warnings are flagged.
    *
    * \retval true if no errors and no warnings are flagged,
    * \retval false if at least one error or one warning was flagged.
    */
    bool isOk() const;
private:
    std::string _ErrorString;
    std::string _WarningString;
    std::string _StackMessage;
};

inline ErrorReader::ErrorReader()
        : _ErrorString( "" )
        , _WarningString( "" )
        , _StackMessage( "" )
{}

inline ErrorReader::~ErrorReader()
{}

inline bool ErrorReader::parseErrorString( const std::string& str )
{
    if ( str.find( "<ftkError>" ) == std::string::npos ||
         str.find( "</ftkError>" ) == std::string::npos )
    {
        std::cerr << "Cannot find root element <ftkError>" << std::endl;
        return false;
    }

    size_t locBegin, locEnd;

    std::vector< std::string > what;
    what.push_back( "errors" );
    what.push_back( "warnings" );
    what.push_back( "messages" );

    std::string* pseudoRef( 0 );

    for ( std::vector< std::string >::const_iterator tagIt( what.begin() );
          tagIt != what.end(); ++tagIt )
    {
        if ( *tagIt == "errors" )
        {
            pseudoRef = &_ErrorString;
        }
        else if ( *tagIt == "warnings" )
        {
            pseudoRef = &_WarningString;
        }
        else if ( *tagIt == "messages" )
        {
            pseudoRef = &_StackMessage;
        }
        if ( str.find( "<" + *tagIt + " />" ) == std::string::npos )
        {
            locBegin = str.find( "<" + *tagIt + ">" );
            locEnd = str.find( "</" + *tagIt + ">" );
            if ( locBegin != std::string::npos &&
                 locEnd != std::string::npos )
            {
                *pseudoRef = str.substr( locBegin + std::string( "<" + *tagIt + ">" ).size(),
                                         locEnd - locBegin - std::string( "<" + *tagIt +  ">" ).size() );
            }
            else
            {
                std::cerr << "Cannot interpret <" << *tagIt << ">" << std::endl;
                return false;
            }
        }
        else
        {
            *pseudoRef = "";
        }

        locBegin = pseudoRef->find( "No errors" );
        if ( locBegin != std::string::npos )
        {
            *pseudoRef = pseudoRef->replace( locBegin, locBegin + strlen( "No errors" ), "" );
        }
    }

    return true;
}

inline bool ErrorReader::hasError( ftkError err ) const
{
    if ( err <= FTK_OK )
    {
        return false;
    }
    else if ( _ErrorString.empty() )
    {
        return false;
    }

    std::stringstream convert;
    convert << int32( err ) << ":";

    return ( _ErrorString.find( convert.str() ) != std::string::npos );
}

inline bool ErrorReader::hasWarning( ftkError war ) const
{
    if ( war >= FTK_OK )
    {
        return false;
    }
    else if ( _ErrorString.empty() )
    {
        return false;
    }

    std::stringstream convert;
    convert << int32( war ) << ":";

    return ( _ErrorString.find( convert.str() ) != std::string::npos );
}

inline bool ErrorReader::isOk() const
{
    return _ErrorString.empty() && _WarningString.empty();
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/** \addtogroup Multithreading helpers.
* \{
*/

/** \brief Mutex interface.
*
* This class defines the interface of the used mutex implementations, used to
* protect access to shared resources from multiple threads.
*/
class Mutex
{
public:
    /// Contructor.
    Mutex();
    /// Destructor, virtual to ensure a proper destruction sequence.
    virtual ~Mutex();
    /** \brief Method locking the mutex.
    *
    * This method tries to lock the mutex for a given amount of time. As soon
    * as the mutex can be locked or once the parameterised time is elapsed the
    * function returns.
    *
    * \param[in] timeout maximal amount of time the method will wait for the
    * mutex to be locked.
    *
    * \retval true if the mutex could be locked,
    * \retval false if the mutex could not be locked in the given amount of
    * time.
    */
    virtual bool lock( uint32 timeout = 0xFFFFFFFF ) = 0;
    /** \brief Method releasing the mutex.
    *
    * This method releases the mutex, allowing it to be locked from another
    * thread.
    */
    virtual void release() = 0;
};

inline Mutex::Mutex()
{}

inline Mutex::~Mutex()
{}

/** \brief Function creating a mutex instance.
*
* This function is responsible for creating an OS-dependant instance of mutex.
*
* \return a pointer on the created mutex specific implementation.
*/
Mutex* createMutex();

// ----------------------------------------------------------------------------

/** \brief Mutex releaser class.
*
* This class provides a convenient way to lock a mutex: at construction the
* mutex is locked, and the release occurs automatically when the object is
* destroyed, e.g. when the releaser gets out of scope.
*/
class Locker
{
public:
    /** \brief Constructor, locks the resource.
    *
    * The constructor may block forever is the mutex is already locked.
    *
    * \param[in] mutex instance of Mutex to lock.
    */
    Locker( Mutex& mutex );
    /** \brief Destructor, releases the resource.
    */
    ~Locker();
private:
    /** \brief Reference on the user Mutex instance.
    */
    Mutex& m;
};

inline Locker::Locker( Mutex& mutex )
        : m( mutex )
{
    m.lock();
}

inline Locker::~Locker()
{
    m.release();
}

// ----------------------------------------------------------------------------

class Thread;
typedef bool ( * OnProcess ) ( const Thread& owner, void* userData );

/** \brief Thread management interface.
*
* This class provides the interface for the thread managing OS-specific
* implementations.
*
* This implementation assumes the function is executed once and then the
* processing is done.
*/
class Thread
{
public:
    /** \brief Constructor.
    *
    * \param[in] onProcess function executed in the thread loop.
    */
    Thread( OnProcess onProcess = 0 );
    /// Destructor
    virtual ~Thread();
    /** \brief Method starting the thread.
    *
    * This method starts the thread and allows to pass parameters to the
    * thread function.
    *
    * \param[in] userData parameters passed to mOnProcess function.*
    *
    * \retval true if the thread could be started,
    * \retval false if an error occurred.
    */
    virtual bool start( void* userData = 0 ) = 0;
    /** \brief Method stopping the thread.
    *
    * This method stops the thread.
    *
    * \retval true if the thread could be stopped,
    * \retval false if an error occurred.
    */
    virtual bool stop() = 0;
    /** \brief Method allowing to determine whether the thread is started.
    *
    * This method checks that the thread has been started.
    *
    * \warning This function is not thread-safe.
    *
    * \retval true if the thread was started,
    * \retval false in all other cases.
    */
    virtual bool isStarted() const = 0;
    /** \brief Method allowing to determine whether the thread is stopped.
    *
    * This method checks that the thread has been started and stopped. If the
    * thread was never started, the result will be \c false.
    *
    * \warning This function is not thread-safe.
    *
    * \retval true if the thread was started \e and stopped,
    * \retval false in all other cases (still running, not started).
    */
    virtual bool isTerminated() = 0;
protected:
    /** \brief Callback function.
    *
    * This function is executed in the separate thread.
    */
    OnProcess mOnProcess;
};

inline Thread::Thread( OnProcess onProcess )
        : mOnProcess( onProcess )
{}

inline Thread::~Thread()
{}

/** \brief Function creating a thread management instance.
*
* This function is responsible for creating an OS-dependant instance of
* thread-managing class.
*
* \param[in] onProcess callback function.
*
* \return a pointer on the created thread specific implementation.
*/
Thread* createThread( OnProcess onProcess );

/**
* \}
*/
