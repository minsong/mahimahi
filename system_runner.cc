/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <cassert>
#include <unistd.h>
#include <numeric>

#include "system_runner.hh"
#include "child_process.hh"
#include "exception.hh"

using namespace std;

void run( const vector< string > & command )
{
    assert( !command.empty() );

    /* copy the arguments to mutable structures */
    vector<char *> argv;
    vector< vector< char > > argv_data;

    for ( auto & x : command ) {
        vector<char> new_str;
        for ( auto & ch : x ) {
            new_str.push_back( ch );
        }
        new_str.push_back( 0 ); /* null-terminate */

        argv_data.push_back( new_str );
    }

    for ( auto & x : argv_data ) {
        argv.push_back( &x[ 0 ] );
    }
    argv.push_back( 0 ); /* null-terminate */

    /* run with empty environment */
    ChildProcess command_process( [&] () {
            if ( execve( &argv[ 0 ][ 0 ], &argv[ 0 ], nullptr ) < 0 ) {
                throw Exception( "execve" );
            }
            return EXIT_FAILURE;
        } );

    while ( !command_process.terminated() ) {
        command_process.wait();
    }

    if ( command_process.exit_status() != 0 ) {
        string command_str = accumulate( command.begin() + 1, command.end(),
                                         command.front(),
                                         []( const string & a, const string & b )
                                         { return a + " " + b; } );
        throw Exception( "`" + command_str + "'", "command returned "
                         + to_string( command_process.exit_status() ) );
    }
}
