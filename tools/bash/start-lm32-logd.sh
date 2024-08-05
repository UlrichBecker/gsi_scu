#!/bin/sh
###############################################################################
##                                                                           ##
##                Shell script starts the LM32-log-daemon.                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      start-lm32-logd.sh                                             ##
## Author:    Ulrich Becker                                                  ##
## Date:      09.07.2024                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
if [ -f /etc/functions ]
then
   . /etc/functions
else
   alias log='echo INFO: '
   alias log_error='echo ERROR: '
fi

#------------------------------------------------------------------------------
# Function returns the german local time-offset in hours, Taking into account
# german-summer and german-standard time.
# Unfortunately the SCU doesn't know about anything of that.
getLocalTimeOffset()
{
   local DATE=$(date -u +%m%d%u%H)
   local month=${DATE:0:2}
   local day=${DATE:2:2}
   local wday=${DATE:4:1}
   local hour=${DATE:5:2}

   # From April to September its always summer time
   if [ "$month" -gt "3" ] && [ "$month" -lt "10" ]
   then
      echo 2
      return 0
   fi

   let sunday=day-wday%7

   # In the month March will happen the switching from normal time to summer time.
   # The switching to summer time will happen at the last Sunday at two o'clock a.m.
   # German time respectively at one o'clock a.m. UTC.
   if [ "$month" = "3" ]
   then
      # It is March, the last sunday couldn't appear before the 25th.
      if [ "$sunday" -lt "25" ]
      then
         # But not yet the last sunday
         echo 1
         return 0
      fi
      if [ "$sunday" = "$day" ]
      then
         if [ "$hour" -lt "1" ]
         then
            # It's the last Sunday but not yet one o'clock a.m. UTC
            echo 1
            return 0
         fi
      fi
      echo 2
      return 0
   fi

   # In the month October will happen the switching from the summer time back to the normal time.
   # The switching to normal time will happen at the last Sunday at two o'clock a.m.
   # German time respectively at 12 o'clock p.m. UTC.
   if [ "$month" = "10" ]
   then
      # It is October, the last sunday couldn't appear before the 25th.
      if [ "$sunday" -lt "25" ]
      then
         # But not yet the last Sunday
         echo 2
         return 0
      fi
      if [ "$sunday" = "$day" ]
      then
         # It is the last Sunday
         if [ "$hour" -lt "0"  ]
         then
            # It's the last Sunday but not yet 12 o'clock p.m. UTC
            echo 2
            return 0
         fi
      fi
   fi

   # from November to February is always normal time
   echo 1
}


ERROR_LOG=/var/log/error.log
COMMON_INIT_DIR=/opt/nfsinit/global/
LM32_LOG_TARGET=/var/log/lm32.log

# Is it the old RAM-disc?
if [ "$(uname -r)" = "3.10.101-rt111-scu03" ]
then
   # Yes, it is the old RAM-disc "scuxl".
   LM32_LOGD=${COMMON_INIT_DIR}tools/scuxl/lm32-logd
else
   # No, it is the new RAM-disc "yocto".
   LM32_LOGD=${COMMON_INIT_DIR}tools/yocto/lm32-logd
fi

if [ -x "$LM32_LOGD" ]
then
   log "Starting LM32-logging-daemon, target: \"${LM32_LOG_TARGET}\""
   $LM32_LOGD -Habd=${LM32_LOG_TARGET} -l$(getLocalTimeOffset) 2>>$ERROR_LOG
else
   log_error "No LM32-log-daemon present"
fi

if [ -f "$ERROR_LOG" ]
then
   log_error $(cat $ERROR_LOG)
fi

#=================================== EOF ======================================
