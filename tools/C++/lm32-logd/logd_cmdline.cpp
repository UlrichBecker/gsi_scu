/*!
 *  @file logd_cmdline.cpp
 *  @brief Commandline parser for the LM32 log daemon.
 *  >>> PvdS <<<
 *  @see https://www-acc.gsi.de/wiki/Frontend/LM32Developments#Handling_of_diagnostic_and_logging_messages_created_by_a_LM32_45Application_via_the_linux_45_application_34lm32_45logd_34
 *  @see https://github.com/UlrichBecker/command_line_option_parser_cpp11
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <scu_env.hpp>
#include <message_macros.hpp>
#include <scu_ddr3_access.hpp>
#include <daq_calculations.hpp>
#include "logd_cmdline.hpp"

using namespace std;
using namespace CLOP;
using namespace Scu;

#define DEFAULT_INTERVAL            1000
#define DEFAULT_MAX_ITEMS            100
#define DEFAULT_MAX_ITEMS_IN_MEMORY 1000
#define DEFAULT_TARGET              "/var/log/lm32.log"

/*! ---------------------------------------------------------------------------
 * @brief Initializing the command line options.
 */
CommandLine::OPT_LIST_T CommandLine::c_optList =
{
#ifdef CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         string name = poParser->getProgramName().substr(poParser->getProgramName().find_last_of('/')+1);
         cout << "<toolinfo>\n"
                 "\t<name>" << name << "</name>\n"
                 "\t<topic>Development, Release, Rollout</topic>\n"
                 "\t<description>Daemon for forwarding log-messages of a LM32-application</description>\n"
                 "\t<usage>" << name << " {SCU-url}";
         for( const auto& pOption: *poParser )
         {
            if( pOption->m_id != 0 )
               continue;
            cout << " [";
            if( pOption->m_shortOpt != '\0' )
            {
               cout << '-' << pOption->m_shortOpt;
               if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                  cout << " ARG";
               if( pOption->m_hasArg == OPTION::OPTIONAL_ARG )
                  cout << " = ARG";
               if( !pOption->m_longOpt.empty() )
                  cout << ", ";
            }
            if( !pOption->m_longOpt.empty() )
            {
               cout << "--" << pOption->m_longOpt;
               if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                  cout << " ARG";
               if( pOption->m_hasArg == OPTION::OPTIONAL_ARG )
                  cout << " = ARG";
            }
            cout << ']';
         }
         cout << "\n\t</usage>\n"
                 "\t<author>Ulrich Becker</author>\n"
                 "\t<autodocversion>1.0</autodocversion>\n"
                 "</toolinfo>"
              << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 1, // will hide this option for autodoc
      .m_shortOpt = '\0',
      .m_longOpt  = "generate_doc_tagged",
      .m_helpText = "Will need from autodoc."
   },
#endif // CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         cout << "\nDaemon for forwarding log-messages of a LM32-application.\n"
                 "(c) 2022 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n\n"
                 "It monitors messages sent by an LM32 application by this function:\n\n"
                 ESC_BOLD
                 "void lm32Log( const unsigned int filter, const char* format, ... );\n\n"
                 ESC_NORMAL
                 "respectively:\n\n"
                 ESC_BOLD
                 "void vLm32log( const unsigned int filter, const char* format, va_list ap );\n\n"
                 ESC_NORMAL
                 "CAUTION: The maximum number of extra parameters after parameter \"format\" per log-item is limited to "
              << static_cast<CommandLine*>(poParser)->getMaxExtraParam() << "\n\n"
                 "Usage on ASL:\n\t"
              << poParser->getProgramName() << " [options] <SCU URL>\n\n"
                 "Usage on SCU:\n\t"
              << poParser->getProgramName() << " [options]\n\n"
                 "The key 'Esc' terminates this program when it runs in non-daemon mode.\n\n"
                 "Example in LM32 application:\n\n"
                 "\t#include <lm32_syslog.h>\n\n"
                 "\tvoid main( void )\n"
                 "\t{\n"
                 "\t   lm32LogInit( 1000 ); // Allocates a maximum of 1000 items in DDR3-buffer respectively SRAM.\n\n"
                 "\t   lm32Log( LM32_LOG_INFO, \"Hello world!\" );\n"
                 "\t}\n\n"
                 "NOTE: The modules " ESC_BOLD "lm32_syslog.c, scu_mmu_lm32.c, scu_mmu.c, circular_index.c\n"
                 ESC_NORMAL "and for SCU3: " ESC_BOLD "scu_ddr3_lm32.c" ESC_NORMAL
                 " or for SCU4: " ESC_BOLD "scu_sram_lm32.c" ESC_NORMAL
                 " has to be linked in the concerned LM32 project.\n\n"
                 "Options:"
              << endl;
         poParser->list( cout );
         cout << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'h',
      .m_longOpt  = "help",
      .m_helpText = "Print this help and exit"
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_verbose = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'v',
      .m_longOpt  = "verbose",
      .m_helpText = "Be verbose."
   },
   {
      OPT_LAMBDA( poParser,
      {
         if( static_cast<CommandLine*>(poParser)->m_verbose )
         {
            cout << "Version: " TO_STRING( VERSION )
                    ", Git revision: " TO_STRING( GIT_REVISION ) << endl;
         }
         else
         {
            cout << TO_STRING( VERSION ) << endl;
         }
         throw runtime_error("");
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'V',
      .m_longOpt  = "version",
      .m_helpText = "Print the software version and exit."
   },
   {
      OPT_LAMBDA( poParser,
      {
         if( !static_cast<CommandLine*>(poParser)->m_isOnScu )
            throw runtime_error( "Daemonizing only on SCU possible!" );

         if( static_cast<CommandLine*>(poParser)->m_isDaemonized )
            throw runtime_error( "Multiple set of daemonizing!" );

         if( poParser->isOptArgPersent() )
            static_cast<CommandLine*>(poParser)->m_logFile = poParser->getOptArg();

         static_cast<CommandLine*>(poParser)->m_isDaemonized = true;
         return 0;
      }),
      .m_hasArg   = OPTION::OPTIONAL_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "daemonize",
      .m_helpText = "Process will run as daemon"
                    " (" ESC_BOLD "d" ESC_NORMAL "isk"
                    " " ESC_BOLD "a" ESC_NORMAL "nd" ESC_BOLD " e"
                    ESC_NORMAL "xecution " ESC_BOLD "mon" ESC_NORMAL "itor)"
                    " if it runs on a SCU.\n"
                    "The optional parameter PARAM can be used to set a target"
                    " logfile.\nIf PARAM not set, then the LM32 messages becomes"
                    " written in Linux-syslog.\n"
                    "Example: " ESC_BOLD "-d=/var/log/lm32.log" ESC_NORMAL
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_noTimestamp = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'n',
      .m_longOpt  = "notime",
      .m_helpText = "Suppresses the output of the timestamp.\n"
                    "This option can be meaningful in combination of the option -c respectively --console ."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_humanTimestamp = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'H',
      .m_longOpt  = "human",
      .m_helpText = "Human readable timestamp."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_isForConsole = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'c',
      .m_longOpt  = "console",
      .m_helpText = "Console mode: line feed \"\\n\" becomes printed.\n"
                    "Otherwise it becomes replaced by space character and \"\\r\" will ignored.\n"
                    "Terminal control sequences after '\\e' (respectively escape sequences)"
                    " will not filtered out es well.\n\n"
                    "NOTE:\n"
                    "It is recommended to use this option in combination with option -n --notime."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_escSequences = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'a',
      .m_longOpt  = "allow-esc-sequences",
      .m_helpText = "Allows terminal control sequences after '\\e' (respectively escape sequences)"
                    " in logging mode.\n"
                    "Otherwise these sequences becomes filtered out."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_interval = readInteger( poParser->getOptArg() );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'I',
      .m_longOpt  = "interval",
      .m_helpText = "PARAM=\"<new poll interval in milliseconds>\"\n"
                    "Overwrites the default interval of " TO_STRING( DEFAULT_INTERVAL )
                    " milliseconds."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint filter = readInteger( poParser->getOptArg() );
         if( filter >= BIT_SIZEOF( FILTER_FLAG_T ) )
         {
            string errStr = "Filter value ";
            errStr += filter;
            errStr += " out of range from 0 to ";
            errStr += (BIT_SIZEOF( FILTER_FLAG_T ) - 1);
            errStr += " !";
            throw std::runtime_error( errStr );
         }
         if( (static_cast<CommandLine*>(poParser)->m_filterFlags & (1 << filter)) != 0 )
            WARNING_MESSAGE( "Filter value " << filter << " is already defined." );
         static_cast<CommandLine*>(poParser)->m_filterFlags |= 1 << filter;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'f',
      .m_longOpt  = "filter",
      .m_helpText = "PARAM=\"<filter value>\"\n"
                    "Setting a filter.\n"
                    "It is possible to specify this option multiple times"
                    " with different values,\n"
                    "from which an OR link is created.\n\n"
                    "E.g. code in LM32:\n"
                    "   lm32Log( 1, \"Log-text A\" );\n"
                    "   lm32Log( 2, \"Log-text B\" );\n"
                    "   lm32Log( 3, \"Log-text C\" );\n\n"
                    "Commandline: -f1 -f3\n"
                    "In this example only \"Log-text A\" and \"Log-text B\""
                    " becomes forwarded.\n\n"
                    "NOTE:\nWhen this option is omitted,\n"
                    "then all log-messages becomes forwarded."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_printFilter = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'p',
      .m_longOpt  = "print-filter",
      .m_helpText = "Prints the filter value at the begin of each item.\n"
                    "That is the first parameter of the LM32 function: "
                    "\"lm32Log\""
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_exit = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'e',
      .m_longOpt  = "exit",
      .m_helpText = "Exit after read, otherwise the program will run in a "
                    "polling loop until the Esc-key has pressed."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_kill = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'k',
      .m_longOpt  = "kill",
      .m_helpText = "Terminates a concurrent running process of this program."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_killOnly = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'K',
      .m_longOpt  = "killonly",
      .m_helpText = "Terminates a concurrent running process of this program"
                    " and exit."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint maxItems = readInteger( poParser->getOptArg() );
         if( maxItems < 1 )
         {
            string errStr = "A maximum of ";
            errStr += to_string( maxItems );
            errStr += " items to read per interval isn't meaningful!";
            throw runtime_error( errStr );
         }
         static_cast<CommandLine*>(poParser)->m_maxItemsPerInterval = maxItems;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'm',
      .m_longOpt  = "maxitems",
      .m_helpText = "PARAM=\"<number of maximum message-items per interval>\"\n"
                    "Overwrites the default number of maximum items per interval of "
                    TO_STRING(DEFAULT_MAX_ITEMS) " with a new value.\n"
                    "That means, this is the maximum number of log-items which becomes\n"
                    "read out and evaluated per poll interval.\n"
                    "It determines also the length of the ehterbone-cycle.\n"
                    "The poll interval can be adjusted by the option -I respectively --interval."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_addBuildId = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'b',
      .m_longOpt  = "add-build-id",
      .m_helpText = "Adds the build identification string of the LM32- application\n"
                    "at the top of the log-file."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_readBuildId = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'B',
      .m_longOpt  = "read-build-id",
      .m_helpText = "Reads the build identification string of the LM32- application\n"
                    "and exit.\n"
                    "NOTE: This option will work in any cases doesn't matter as the LM32- application\n"
                    "supports the logging or not."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_burstLimit = readInteger( poParser->getOptArg() );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'u',
      .m_longOpt  = "burst-limit",
      .m_helpText = "PARAM specifies the number of 64-bit words at which DDR3-RAM"
                    " is read in burst mode.\n"
                    "If the value is zero, then the DDR3-RAM is always read out in"
                    " burst mode.\n"
                    "Burst mode is never used by default.\n"
                    "If the DDR3-RAM is not involved in the data transfer,"
                    " then this option has no effect."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_maxItems = readInteger( poParser->getOptArg() );
         if( static_cast<CommandLine*>(poParser)->m_maxItems == 0 )
            throw runtime_error( "Number of zero items isn't allowed!" );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'M',
      .m_longOpt  = "max-log",
      .m_helpText = "PARAM specifies the maximum number of log messages in the SCU-RAM FiFo\n"
                    "NOTE: This option has only an effect when the concerning memory segment\n"
                    "      is not already allocated.\n"
                    "The default value is: " TO_STRING(DEFAULT_MAX_ITEMS_IN_MEMORY) " message items."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_doReset = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'r',
      .m_longOpt  = "reset",
      .m_helpText = "Makes a fifo-reset respectively a reinitialization"
                    " during start of this application.\n"
                    "CAUTION: Possible stored log-data will be lost!\n"
                    "NOTE: When the memory already has been allocated by"
                    " the application \"mem-mon\" so this option becomes mandatory."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_timeInUtc = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'U',
      .m_longOpt  = "utc",
      .m_helpText = "Converts the white rabbit timestamp in to universal time (UTC)."
   },
   {
      OPT_LAMBDA( poParser,
      {
         const int lto = readInteger( poParser->getOptArg() );
         if( (lto < -12) || (lto > +12) )
         {
            throw runtime_error( "Local time offset is out of the alowed range of +/- 12h!" );
         }
         static_cast<CommandLine*>(poParser)->m_localTimeOffset = lto * daq::NANOSECS_PER_HOUR;
         DEBUG_MESSAGE( "LTO: " << static_cast<CommandLine*>(poParser)->m_localTimeOffset );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'l',
      .m_longOpt  = "localTimeOffset",
      .m_helpText = "PARAM specifies the time zone in hours if the timestamps should be given in the current local time.\n"
                    "If this option is used, a conversion from TAI to UTC is also done before.\n"
                    "Unfortunately, this option is necessary if lm32-logd is invoked on the SCU, as it only knows UTC."
   }
}; // CommandLine::c_optList

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
*/
int CommandLine::readInteger( const string& roStr )
{
   int retVal;
   try
   {
      retVal = stoi( roStr, nullptr, (roStr[0] == '0' && roStr[1] == 'x')? 16 : 10 );
   }
   catch( std::exception& e )
   {
      std::string errStr = "Integer number is expected and not that: \"";
      errStr += roStr;
      errStr += "\" !";
      throw std::runtime_error( errStr );
   }
   return retVal;
}

/*! ---------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,m_verbose( false )
   ,m_isOnScu( isRunningOnScu() )
   ,m_noTimestamp( false )
   ,m_humanTimestamp( false )
   ,m_isForConsole( false )
   ,m_escSequences( false )
   ,m_printFilter( false )
   ,m_exit( false )
   ,m_kill( false )
   ,m_killOnly( false )
   ,m_isDaemonized( false )
   ,m_addBuildId( false )
   ,m_readBuildId( false )
   ,m_doReset( false )
   ,m_timeInUtc( false )
   ,m_interval( DEFAULT_INTERVAL )
   ,m_maxItemsPerInterval( DEFAULT_MAX_ITEMS )
   ,m_burstLimit( Ddr3Access::NEVER_BURST )
   ,m_maxItems( DEFAULT_MAX_ITEMS_IN_MEMORY )
   ,m_localTimeOffset( 0 )
   ,m_filterFlags( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_isOnScu )
      m_scuUrl = "dev/wbm0";
   add( c_optList );
   sortShort();

   operator()();
}

/*! ---------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_isOnScu )
   {
      WARNING_MESSAGE( "Program is running on SCU, therefore the argument \""
                       << getArgVect()[getArgIndex()]
                       << "\" becomes replaced by \""
                       << m_scuUrl << "\"!" );
      return 1;
   }

   if( !m_scuUrl.empty() )
   {
      ERROR_MESSAGE( "Only one argument is allowed!" );
      ::exit( EXIT_FAILURE );
   }

   m_scuUrl = getArgVect()[getArgIndex()];
   if( m_scuUrl.find( "tcp/" ) == string::npos )
         m_scuUrl = "tcp/" + m_scuUrl;

   return 1;
}

/*! ---------------------------------------------------------------------------
 */
std::string& CommandLine::operator()( void )
{
   DEBUG_MESSAGE( "Parsing of commandline" );
   if( PARSER::operator()() < 0 )
      ::exit( EXIT_FAILURE );

   if( !m_isOnScu && m_scuUrl.empty() )
   {
      ERROR_MESSAGE( "Missing SCU URL" );
      ::exit( EXIT_FAILURE );
   }

   if( m_humanTimestamp && m_noTimestamp )
      WARNING_MESSAGE( "Timestamp will not printed, therefore"
                       " the option for human readable timestamp"
                       " has no effect!" );

   if( isVerbose() && !m_logFile.empty() )
   {
      std::cout << "Log-target is: \"" << m_logFile << "\"." << std::endl;
   }

   return m_scuUrl;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedShortOption( char unrecognized )
{
   ERROR_MESSAGE( "Unknown short option: \"-" << unrecognized << "\"" );
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedLongOption( const std::string& unrecognized )
{
   ERROR_MESSAGE( "Unknown long option: \"--" << unrecognized << "\"" );
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorShortMissingRequiredArg( void )
{
   ERROR_MESSAGE( "Missing argument of option: -" << getCurrentOption()->m_shortOpt );
   return -1;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorLongMissingRequiredArg( void )
{
   ERROR_MESSAGE( "Missing argument of option: --" << getCurrentOption()->m_longOpt );
   return -1;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorShortOptionalArg( void )
{
   ERROR_MESSAGE( "Missing argument after '=' of option: -" << getCurrentOption()->m_shortOpt );
   return -1;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorlongOptionalArg( void )
{
   ERROR_MESSAGE( "Missing argument after '=' of option --" << getCurrentOption()->m_longOpt );
   return -1;
}

//================================== EOF ======================================
