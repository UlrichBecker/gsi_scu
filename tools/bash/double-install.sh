# !/bin/bash
###############################################################################
##                                                                           ##
##  Install-script for installing a executable file on ASL and SCU via NFS   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      double-install.sh                                              ##
## Author:    Ulrich Becker                                                  ##
## Date:      25.11.2022                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################
ESC_ERROR="\e[1m\e[31m"
ESC_SUCCESS="\e[1m\e[32m"
ESC_NORMAL="\e[0m"

PUBLIC_ASL_TARGET_DIR=/common/usr/cscofe/bin/
PUBLIC_SCU_TARGET_DIR=/common/export/nfsinit/global/tools/

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
Install-script for installing a executable file on ASL and SCU via NFS.
Usage:
  $(basename $0) [Options] <executable file>


Options:
-h, --help
   This help and exit.

-v, --verbose
   Be verbose

__EOH__
   exit 0
}

#==============================================================================
BE_VERBOSE=false
while [ "${1:0:1}" = "-" ]
do
   A=${1#-}
   while [ -n "$A" ]
   do
      case ${A:0:1} in
         "h")
            printHelp
         ;;
         "v")
            BE_VERBOSE=true
         ;;
         "-")
            B=${A#*-}
            case ${B%=*} in
               "help")
                  printHelp
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

if [ "${HOSTNAME:0:5}" != "asl74" ]
then
   die "Script can run on ASL74[0-4] only!"
fi

if [ ! -n "$1" ]
then
   die "Missing executable file!"
fi

if [ ! -x "$1" ]
then
   die "File \"$1\" is not executable!"
fi

if [ ! -d "$PUBLIC_ASL_TARGET_DIR" ]
then
   die "ASL-target directory \"$PUBLIC_ASL_TARGET_DIR\" not found"
fi

if [ ! -d "$PUBLIC_SCU_TARGET_DIR" ]
then
   die "SCU-target directory \"$PUBLIC_SCU_TARGET_DIR\" not found"
fi

if $BE_VERBOSE
then
   echo "Copy \"$1\" to \"$PUBLIC_SCU_TARGET_DIR\""
fi
cp -u $1 $PUBLIC_SCU_TARGET_DIR

if $BE_VERBOSE
then
   echo "Creating a symbolic link \"$PUBLIC_ASL_TARGET_DIR$(basename $1)\" -> \"$PUBLIC_SCU_TARGET_DIR$(basename $1)\""
fi
ln -sf $PUBLIC_SCU_TARGET_DIR$(basename $1) $PUBLIC_ASL_TARGET_DIR

#=================================== EOF ======================================
