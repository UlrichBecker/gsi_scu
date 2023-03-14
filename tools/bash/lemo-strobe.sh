# !/bin/bash
###############################################################################
##                                                                           ##
##     Shellscript generates a trigger-pulse on LEMO-connector B1 or B2      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      lemo-strobe.sh                                                 ##
## Author:    Ulrich Becker                                                  ##
## Date:      14.03.2023                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
PROG_NAME=${0##*/}
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}


#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Usage $PROG_NAME [OPTION] <1-2>

Generates a trigger pulse on LEMO connector B1 or B2. E.g. for triggering a interrupt
for DAQ high-resolution if connected on B1 or B2.

Options:
   -h, --help
   This help and exit.

   -d<delay>
   Minimum duration in microseconds for logic one on concerned LEMO.
__EOH__
   exit 0
}

#------------------------------------------------------------------------------
docTagged()
{
   cat << __EOH__
<toolinfo>
   <name>$(basename $0)</name>
   <topic>Helpers</topic>
   <description>Generates a logic one on LEMO connector B1 or B2</description>
   <usage>
      $(basename $0) [option] {LEMO number 1 or 2}
   </usage>
   <author>ubecker@gsi.de</author>
   <tags></tags>
   <version>1.0</version>
   <documentation>
   </documentation>
   <environment>scu</environment>
   <requires>saft-io-ctl</requires>
   <autodocversion>1.0</autodocversion>
   <compatibility></compatibility>
</toolinfo>"
__EOH__
   exit 0
}


#==============================================================================

DELAY=0
while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "h")
            printHelp
         ;;
         "d")
            DELAY=${A:1}
            A=""
         ;;
         "-")
            B=${A#*-}
            case ${B%=*} in
               "help")
                  printHelp
               ;;
               "generate_doc_tagged")
                  docTagged
               ;;
               *)
                  die "Unknown long option \"-${A}\"!"
               ;;
            esac
            A=""
         ;;
         *)
            die "Unknown short option: \"${A:0:1}\"!"
         ;;
      esac
      A=${A#?}
   done
   shift
done

if [ ! -n "$1" ]
then
   die "Missing argument, I don't know which LIMO, B1 or B2!"
fi

case $1 in
   "1")
   ;;
   "2")
   ;;
   *)
      die "Lemo $1 not present!"
   ;;
esac

IO_CTL=$(which saft-io-ctl 2>/dev/null)
if [ ! -x "$IO_CTL" ]
then
   die "Program saft-io-ctl not found!"
fi

$IO_CTL tr0 -n B$1 -o1
$IO_CTL tr0 -n B$1 -d0
$IO_CTL tr0 -n B$1 -d1
usleep $DELAY
$IO_CTL tr0 -n B$1 -d0

#=================================== EOF ======================================
