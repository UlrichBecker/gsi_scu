#!/bin/sh
###############################################################################
##                                                                           ##
##            Opening of three work-consoles on the target SCU               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:       workscu.sh                                                    ##
## Author:     Ulrich Becker                                                 ##
## Date:       26.01.2023                                                    ##
## Copyright:  GSI Helmholtz Centre for Heavy Ion Research GmbH              ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

TOOL_SCRIPT=__scuTools.sh
LOG_SCRIPT=__LM32Log.sh
CON_SCRIPT=__scuCon.sh

SCU_TOOL_DIR="/opt/nfsinit/global/tools/"

#------------------------------------------------------------------------------
die()
{
   echo -e $ESC_ERROR"ERROR: $@"$ESC_NORMAL 1>&2
   exit 1;
}

#------------------------------------------------------------------------------
checkTarget()
{
   [ ! -n "$1" ] && die "Missing target URL!"
   ping -c1 $1 2>/dev/null 1>/dev/null
   [ "$?" != '0' ] && die "Target \"$1\" not found!"
}

#------------------------------------------------------------------------------
printHelp()
{
   cat << __EOH__
Script opens three terminals on the concerning SCU:
   1) Terminal in the tool-directory;
   2) Terminal showing the lm32-log
   3) LM32-console

Usage: $0 [option] <scu-name>

Options:
-h, --help 
   This help and exit.

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
   <description>Opens three terminals on the concerning SCU</description>
   <usage>
      $(basename $0) [option] <SCU-name>
   </usage>
   <author>ubecker@gsi.de</author>
   <tags></tags>
   <version>1.0</version>
   <documentation>
      Opens three terminals on the concerning SCU:
      1) LM32 console
      2) LM32 log output
      3) I/O-terminal on SCU tool directory
   </documentation>
   <environment>scu</environment>
   <requires>
      on KDE: konsole, or on XFCE: xfce4-terminal
      on SCU: eb-console, slogf.sh
   </requires>
   <autodocversion>1.0</autodocversion>
   <compatibility></compatibility>
</toolinfo>"
__EOH__
   exit 0
}

#------------------------------------------------------------------------------
outFTools()
{
   cat << __EOT__
#!/bin/sh
ssh -t root@$SCU_URL "export ENV=${SCU_TOOL_DIR}fe_scripts/fe_environment.sh; cd ${SCU_TOOL_DIR}; sh -i"
__EOT__
}

#------------------------------------------------------------------------------
outFLog()
{
   cat << __EOT__
#!/bin/sh
ssh -t root@$SCU_URL "${SCU_TOOL_DIR}slogf.sh; sh -i"
__EOT__
}

#------------------------------------------------------------------------------
outLm32Console()
{
   cat << __EOT__
#!/bin/sh
ssh -t root@$SCU_URL "eb-console dev/wbm0; sh -i"
__EOT__
}

#------------------------------------------------------------------------------

while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "h")
            printHelp
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
   die "Missing SCU name!"
fi


if [ "${HOSTNAME:0:5}" = "asl74" ]
then
   #
   # Script is running on ASL-cluster
   #
   SCU_URL=$1
   checkTarget $SCU_URL

   if [ -x "$(which konsole)" ]
   then
      CONSOLE="konsole -e"
   elif [ -x "$(which xfce4-terminal)" ]
   then
      CONSOLE="xfce4-terminal -x"
   else
      die "Console application not found!"
   fi

   outFTools >> $TOOL_SCRIPT
   chmod +x $TOOL_SCRIPT

   outFLog >> $LOG_SCRIPT
   chmod +x $LOG_SCRIPT

   outLm32Console >> $CON_SCRIPT
   chmod +x $CON_SCRIPT

   $CONSOLE ./$TOOL_SCRIPT &
   $CONSOLE ./$LOG_SCRIPT &
   $CONSOLE ./$CON_SCRIPT &

   sleep 3
   rm ./$TOOL_SCRIPT
   rm ./$LOG_SCRIPT
   rm ./$CON_SCRIPT
else
   #
   # Script ts running on remote-PC
   #
   if [ ! -n "$GSI_USERNAME" ]
   then
      echo "GSI username: "
      read GSI_USERNAME
   fi
   if [ ! -n "$ASL_NO" ]
   then
      ASL_NO=744
   fi
   ASL_URL=asl${ASL_NO}.acc.gsi.de
   echo "Recursive call on asl${ASL_NO}:"
   ssh -x -t ${GSI_USERNAME}@${ASL_URL} $(basename $0) $1
fi

#=================================== EOF ======================================
