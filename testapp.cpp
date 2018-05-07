// ============================================================================
/*!
 *
 *   This file is part of the ATRACSYS fusionTrack library.
 *   Copyright (C) 2003-2014 by Atracsys LLC. All rights reserved.
 *
 *   \file ftk2_AcquisitionBasic.cpp
 *   \brief Demonstrate basic acquisition
 *
 *   NOTE  THAT THIS SAMPLE WILL RETURN NO OUTPUT IF YOU DO NOT
 *   PLACE A MS2 GEOMETRY IN FRONT OF THE LOCALIZER!
 *
 *   This sample aims to present the following driver features:
 *   - Open/close the driver
 *   - Enumerate devices
 *   - Load a geometry
 *   - Acquire pose (translation + rotation) data of loaded geometries
 *
 *   How to compile this example:
 *   - Install the fusionTrack driver (see documentation)
 *   - Add fusionTrack "include/api/" directory in your project
 *   - Add this file in your project
 *   - Add the fusionTrack.lib in your project
 *
 *   How to run this example:
 *   - Install the fusionTrack driver (see documentation)
 *   - Switch on device
 *   - Run the resulting executable
 *   - Expose a marker (with a MS2 geometry) in front of the localizer
 *
 */
// ============================================================================

#include "lib/helpers.hpp"
#include "lib/geometryHelper.hpp"

#include <iomanip>
#include <iostream>

#ifdef FORCED_DEVICE_DLL_PATH
#include <Windows.h>
#endif

using namespace std;
static const unsigned ENABLE_ONBOARD_PROCESSING_OPTION = 6000;
static const unsigned SENDING_IMAGES_OPTION = 6003;

ftkLibrary lib = 0;
uint64 sn( 0uLL );

// ---------------------------------------------------------------------------
// main function

int main( int argc, char** argv )
{
    // -----------------------------------------------------------------------
    // Defines where to find Atracsys SDK dlls when FORCED_DEVICE_DLL_PATH is
    // set.
#ifdef FORCED_DEVICE_DLL_PATH
    SetDllDirectory( (LPCTSTR) FORCED_DEVICE_DLL_PATH );
#endif

    string geomFile( "geometry002.ini" );

    if ( argc < 2 )
    {
        cerr << "No input geometry given, will try to load the default one: "
             << geomFile << endl;
        cout << "Usage: " << argv[ 0u ] << " geometry_file" << endl;
        cout << "The file will be sought in the current directory and in "
             << "the installation folder (windows only)." << endl;
    }
    else
    {
        geomFile = argv[ 1u ];
    }

    // ------------------------------------------------------------------------
    // Initialize driver

    lib = ftkInit();
    if ( lib == 0 )
    {
        error( "Cannot initialize driver" );
    }

    // ------------------------------------------------------------------------
    // Retrieve the device

    DeviceData device( retrieveLastDevice( lib ) );
    uint64 sn( device.SerialNumber );

    // ------------------------------------------------------------------------
    // When using a spryTrack, onboard processing of the images is preferred.
    // Sending of the images is disabled so that the sample operates on a USB2
    // connection
    if (DEV_SPRYTRACK_180 == device.Type)
    {
        cout << "Enable onboard processing" << endl;
        if ( ftkSetInt32( lib, sn, ENABLE_ONBOARD_PROCESSING_OPTION, 1 ) != FTK_OK )
        {
            error( "Cannot process data directly on the SpryTrack." );
        }

        cout << "Disable images sending" << endl;
        if ( ftkSetInt32( lib, sn, SENDING_IMAGES_OPTION, 0 ) != FTK_OK )
        {
            error( "Cannot disable images sending on the SpryTrack." );
        }
    }

    // ------------------------------------------------------------------------
    // Set geometry

    ftkGeometry geom;

    switch ( loadGeometry( lib, sn, geomFile, geom ) )
    {
        case 1:
            cout << "Loaded from installation directory." << endl;

        case 0:
            if ( FTK_OK != ftkSetGeometry( lib, sn, &geom ) )
            {
                checkError( lib );
            }
            break;

        default:

            cerr << "Error, cannot load geometry file '"
                 << geomFile << "'." << endl;
            if ( FTK_OK != ftkClose( &lib ) )
            {
                checkError( lib );
            }

#ifdef ATR_WIN
            cout << endl << " *** Hit a key to exit ***" << endl;
            waitForKeyboardHit();
#endif
            return 1;
    }

    // ------------------------------------------------------------------------
    // Initialize the frame to get marker pose

    ftkFrameQuery* frame = ftkCreateFrame();

    if ( frame == 0 )
    {
        error( "Cannot create frame instance" );
    }

    ftkError err( ftkSetFrameOptions( false, false, 16u, 16u, 0u, 16u,
                                      frame ) );

    if ( err != FTK_OK )
    {
        ftkDeleteFrame( frame );
        checkError( lib );
    }

    uint32 counter( 10u );

    cout.setf( std::ios::fixed, std::ios::floatfield );

    for ( uint32 u( 0u ), i; u < 100u; ++u )
    {

        /* block up to 100 ms if next frame is not available*/
        err = ftkGetLastFrame( lib, sn, frame, 100u );
        if ( err != FTK_OK )
        {
            cout << ".";
            continue;
        }

        switch ( frame->markersStat )
        {
            case QS_WAR_SKIPPED:
                ftkDeleteFrame( frame );
                cerr << "marker fields in the frame are not set correctly" << endl;
                checkError( lib );

            case QS_ERR_INVALID_RESERVED_SIZE:
                ftkDeleteFrame( frame );
                cerr << "frame -> markersVersionSize is invalid" << endl;
                checkError( lib );

            default:
                ftkDeleteFrame( frame );
                cerr << "invalid status" << endl;
                checkError( lib );

            case QS_OK:
                break;
        }

        if ( frame->markersCount == 0u )
        {
            cout << ".";
            sleep( 1000L );
            continue;
        }

        if ( frame->markersStat == QS_ERR_OVERFLOW )
        {
            cerr <<
                 "WARNING: marker overflow. Please increase cstMarkersCount"
                 << endl;
        }

        for ( i = 0u; i < frame->markersCount; ++i )
        {
            cout << endl;
            cout.precision( 2u );
            cout << "geometry " << frame->markers[ i ].geometryId
                 << ", trans (" << frame->markers[ i ].translationMM[ 0u ]
                 << " " << frame->markers[ i ].translationMM[ 1u ]
                 << " " << frame->markers[ i ].translationMM[ 2u ]
                 << "), error ";
            cout.precision( 3u );
            cout << frame->markers[ i ].registrationErrorMM << endl;
        }

        if ( --counter == 0uLL )
        {
            break;
        }

        sleep( 1000L );
    }

    if ( counter != 0u )
    {
        cout << endl << "Acquisition loop aborted after too many vain trials"
             << endl;
    }
    else
    {
        cout << "\tSUCCESS" << endl;
    }

    // ------------------------------------------------------------------------
    // Close driver

    ftkDeleteFrame( frame );

    if ( FTK_OK != ftkClose( &lib ) )
    {
        checkError( lib );
    }

#ifdef ATR_WIN
    cout << endl << " *** Hit a key to exit ***" << endl;
    waitForKeyboardHit();
#endif
    return EXIT_SUCCESS;
}
