# #!/bin/bash
# Script reg_test.sh for all DAQ Regs in ADDAC2 Implementation # Written by kkaiser  Initial Version 1.0 2017Nov21


[ -z $BASH ] || shopt -s expand_aliases
alias BEGINCOMMENT="if [ ]; then"
alias ENDCOMMENT="fi"



ebFind()
{
   eb-find ${1} ${2} ${3}
   if [ "$?" != "0" ]
   then
      echo -e "\e[31m\e[1mERROR: eb-find ${1} ${2} ${3}\e[0m" 1>&2
      exit 1
   fi
}

ebWrite()
{
   eb-write ${1} ${2} ${3}
   if [ "$?" != "0" ]
   then
      echo -e "\e[31m\e[1mERROR: eb-write ${1} ${2} ${3}\e[0m" 1>&2
      exit 1
   fi
}

ebRead()
{
   eb-read ${1} ${2} ${3}
   if [ "$?" != "0" ]
   then
      echo -e "\e[31m\e[1mERROR: eb-read ${1} ${2} ${3}\e[0m" 1>&2
      exit 1
   fi
}


scuname=0035

URL="tcp/scuxl${scuname}"

scuslave_baseadr=$(ebFind ${URL} 0x651 0x9602eb6f); [ "$?" != "0" ] && exit 1

echo "addr: $scuslave_baseadr"

#exit 0

i=7
# eb-sflash -s 2 tcp/scuxl0035.acc -t

sleep 2


echo "----------------------------------------------------------------------"
  CtrlReg_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4000)) ))" )" 
  CtrlReg_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4002)) ))" )" 
  CtrlReg_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4004)) ))" )" 
  CtrlReg_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4006)) ))" )" 
  echo "Adresse CtrlReg 1="$CtrlReg_Ch1
  echo "Adresse CtrlReg 2="$CtrlReg_Ch2
  echo "Adresse CtrlReg 3="$CtrlReg_Ch3
  echo "Adresse CtrlReg 4="$CtrlReg_Ch4

exit 0

echo "----------------------------------------------------------------------"
  TrigLW_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4020)) ))" )" 
  TrigLW_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4022)) ))" )" 
  TrigLW_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4024)) ))" )" 
  TrigLW_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4026)) ))" )" 
  echo "Adresse TrigLW_Ch 1="$TrigLW_Ch1
  echo "Adresse TrigLW_Ch 2="$TrigLW_Ch2
  echo "Adresse TrigLW_Ch 3="$TrigLW_Ch3
  echo "Adresse TrigLW_Ch 4="$TrigLW_Ch4 echo "----------------------------------------------------------------------"
  TrigHW_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4040)) ))" )" 
  TrigHW_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4042)) ))" )" 
  TrigHW_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4044)) ))" )" 
  TrigHW_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4046)) ))" )" 
  echo "Adresse TrigHW_Ch 1="$TrigHW_Ch1
  echo "Adresse TrigHW_Ch 2="$TrigHW_Ch2
  echo "Adresse TrigHW_Ch 3="$TrigHW_Ch3
  echo "Adresse TrigHW_Ch 4="$TrigHW_Ch4 echo "----------------------------------------------------------------------"
  TrigDly_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4060)) ))" )" 
  TrigDly_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4062)) ))" )" 
  TrigDly_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4064)) ))" )" 
  TrigDly_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4066)) ))" )" 
  echo "Adresse TrigDly_Ch 1="$TrigDly_Ch1
  echo "Adresse TrigDly_Ch 2="$TrigDly_Ch2
  echo "Adresse TrigDly_Ch 3="$TrigDly_Ch3
  echo "Adresse TrigDly_Ch 4="$TrigDly_Ch4 echo "----------------------------------------------------------------------"
  PMDat_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4080)) ))" )" 
  PMDat_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4082)) ))" )" 
  PMDat_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4084)) ))" )" 
  PMDat_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4086)) ))" )" 
  echo "Adresse PMDat 1="$PMDat_Ch1
  echo "Adresse PMDat 2="$PMDat_Ch2
  echo "Adresse PMDat 3="$PMDat_Ch3
  echo "Adresse PMDat 4="$PMDat_Ch4
echo "----------------------------------------------------------------------"
  DaqDat_Ch1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40A0)) ))" )" 
  DaqDat_Ch2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40A2)) ))" )" 
  DaqDat_Ch3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40A4)) ))" )" 
  DaqDat_Ch4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40A6)) ))" )" 
  echo "Adresse DaqDat 1="$DaqDat_Ch1
  echo "Adresse DaqDat 2="$DaqDat_Ch2
  echo "Adresse DaqDat 3="$DaqDat_Ch3
  echo "Adresse DaqDat 4="$DaqDat_Ch4
echo "----------------------------------------------------------------------"
  DaqInts="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40c0)) ))" )" 
  HiResInts="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40c2)) ))" )" 
  TS_Counter_Wd1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40c4)) ))" )" 
  TS_Counter_Wd2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40c6)) ))" )" 
  TS_Counter_Wd3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40c8)) ))" )" 
  TS_Counter_Wd4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40ca)) ))" )" 
  TS_Cntr_Tag_LW="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40cc)) ))" )" 
  TS_Cntr_Tag_HW="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40ce)) ))" )" 
  echo "Adresse DaqInts ="$DaqInts
  echo "Adresse HiResInts ="$HiResInts
  echo "Adresse TS_Counter_Wd1 ="$TS_Counter_Wd1
  echo "Adresse TS_Counter_Wd2 ="$TS_Counter_Wd2
  echo "Adresse TS_Counter_Wd3 ="$TS_Counter_Wd3
  echo "Adresse TS_Counter_Wd4 ="$TS_Counter_Wd4
  echo "Adresse TS_Cntr_Tag_LW ="$TS_Cntr_Tag_LW
  echo "Adresse TS_Cntr_Tag_HW ="$TS_Cntr_Tag_HW

echo "----------------------------------------------------------------------"
  DaqFifoWords1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40e0)) ))" )" 
  DaqFifoWords2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40e2)) ))" )" 
  DaqFifoWords3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40e4)) ))" )" 
  DaqFifoWords4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x40e6)) ))" )" 
  echo "Adresse DaqFifoWords 1="$DaqFifoWords1
  echo "Adresse DaqFifoWords 2="$DaqFifoWords2
  echo "Adresse DaqFifoWords 3="$DaqFifoWords3
  echo "Adresse DaqFifoWords 4="$DaqFifoWords4 echo "----------------------------------------------------------------------"
  PMFifoWords1="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4100)) ))" )" 
  PMFifoWords2="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4102)) ))" )" 
  PMFifoWords3="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4104)) ))" )" 
  PMFifoWords4="0x$( printf "%X\n" $"$((10#$(($scuslave_baseadr+0x20000*i+0x4106)) ))" )" 
  echo "Adresse PMFifoWords 1="$PMFifoWords1
  echo "Adresse PMFifoWords 2="$PMFifoWords2
  echo "Adresse PMFifoWords 3="$PMFifoWords3
  echo "Adresse PMFifoWords 4="$PMFifoWords4

echo "----------------------------------------------------------------------"
#echo "   Write all daq registers with 0x0000 and read them back             "

data=0x0000

ebWrite  ${URL}   ${CtrlReg_Ch1}/2 $data
ebWrite  ${URL}   ${CtrlReg_Ch2}/2 $data
ebWrite  ${URL}   ${CtrlReg_Ch3}/2 $data
ebWrite  ${URL}   ${CtrlReg_Ch4}/2 $data
echo "lesewert CtrlReg_Ch1 =" $(ebRead  ${URL}   ${CtrlReg_Ch1}/2)
echo "lesewert CtrlReg_Ch2 =" $(ebRead  ${URL}   ${CtrlReg_Ch2}/2)
echo "lesewert CtrlReg_Ch3 =" $(ebRead  ${URL}   ${CtrlReg_Ch3}/2)
echo "lesewert CtrlReg_Ch4 =" $(ebRead  ${URL}   ${CtrlReg_Ch4}/2)
echo "----------------------------------------------------------------------"
ebWrite  ${URL}   ${TrigLW_Ch1}/2 $data
ebWrite  ${URL}   ${TrigLW_Ch2}/2 $data
ebWrite  ${URL}   ${TrigLW_Ch3}/2 $data
ebWrite  ${URL}   ${TrigLW_Ch4}/2 $data
echo "lesewert TrigLW_Ch1 =" $(ebRead  ${URL}   ${TrigLW_Ch1}/2)
echo "lesewert TrigLW_Ch2 =" $(ebRead  ${URL}   ${TrigLW_Ch2}/2)
echo "lesewert TrigLW_Ch3 =" $(ebRead  ${URL}   ${TrigLW_Ch3}/2)
echo "lesewert TrigLW_Ch4 =" $(ebRead  ${URL}   ${TrigLW_Ch4}/2)
echo "----------------------------------------------------------------------"
ebWrite  ${URL}   ${TrigHW_Ch1}/2 $data
ebWrite  ${URL}   ${TrigHW_Ch2}/2 $data
ebWrite  ${URL}   ${TrigHW_Ch3}/2 $data
ebWrite  ${URL}   ${TrigHW_Ch4}/2 $data
echo "lesewert TrigHW_Ch1 =" $(ebRead  ${URL}   ${TrigHW_Ch1}/2)
echo "lesewert TrigHW_Ch2 =" $(ebRead  ${URL}   ${TrigHW_Ch2}/2)
echo "lesewert TrigHW_Ch3 =" $(ebRead  ${URL}   ${TrigHW_Ch3}/2)
echo "lesewert TrigHW_Ch4 =" $(ebRead  ${URL}   ${TrigHW_Ch4}/2)
echo "----------------------------------------------------------------------"
ebWrite  ${URL}   ${TrigDly_Ch1}/2 $data
ebWrite  ${URL}   ${TrigDly_Ch2}/2 $data
ebWrite  ${URL}   ${TrigDly_Ch3}/2 $data
ebWrite  ${URL}   ${TrigDly_Ch4}/2 $data
echo "lesewert TrigDly_Ch1 =" $(ebRead  ${URL}   ${TrigDly_Ch1}/2)
echo "lesewert TrigDly_Ch2 =" $(ebRead  ${URL}   ${TrigDly_Ch2}/2)
echo "lesewert TrigDly_Ch3 =" $(ebRead  ${URL}   ${TrigDly_Ch3}/2)
echo "lesewert TrigDly_Ch4 =" $(ebRead  ${URL}   ${TrigDly_Ch4}/2)
echo "----------------Remember: Fifo Words are read only--------------------"
ebWrite  ${URL}   ${PMDat_Ch1}/2 $data
ebWrite  ${URL}   ${PMDat_Ch2}/2 $data
ebWrite  ${URL}   ${PMDat_Ch3}/2 $data
ebWrite  ${URL}   ${PMDat_Ch4}/2 $data
echo "lesewert PMDat_Ch1 =" $(ebRead  ${URL}   ${PMDat_Ch1}/2)
echo "lesewert PMDat_Ch2 =" $(ebRead  ${URL}   ${PMDat_Ch2}/2)
echo "lesewert PMDat_Ch3 =" $(ebRead  ${URL}   ${PMDat_Ch3}/2)
echo "lesewert PMDat_Ch4 =" $(ebRead  ${URL}   ${PMDat_Ch4}/2)
echo "----------------Remember: Fifo Words are read only--------------------"
ebWrite  ${URL}   ${DaqDat_Ch1}/2 $data
ebWrite  ${URL}   ${DaqDat_Ch2}/2 $data
ebWrite  ${URL}   ${DaqDat_Ch3}/2 $data
ebWrite  ${URL}   ${DaqDat_Ch4}/2 $data
echo "lesewert DaqDat_Ch1 =" $(ebRead  ${URL}   ${DaqDat_Ch1}/2)
echo "lesewert DaqDat_Ch2 =" $(ebRead  ${URL}   ${DaqDat_Ch2}/2)
echo "lesewert DaqDat_Ch3 =" $(ebRead  ${URL}   ${DaqDat_Ch3}/2)
echo "lesewert DaqDat_Ch4 =" $(ebRead  ${URL}   ${DaqDat_Ch4}/2)
echo "---------------Remember: Ints only 4 bits used--------------------------"
ebWrite  ${URL}   ${DaqInts}/2 $data
ebWrite  ${URL}   ${HiResInts}/2 $data
ebWrite  ${URL}   ${TS_Counter_Wd1}/2 $data
ebWrite  ${URL}   ${TS_Counter_Wd2}/2 $data
ebWrite  ${URL}   ${TS_Counter_Wd3}/2 $data
ebWrite  ${URL}   ${TS_Counter_Wd4}/2 $data
ebWrite  ${URL}   ${TS_Cntr_Tag_LW}/2 $data
ebWrite  ${URL}   ${TS_Cntr_Tag_HW}/2 $data
echo "lesewert DaqInts =" $(ebRead  ${URL}   ${DaqInts}/2)
echo "lesewert HiResInts =" $(ebRead  ${URL}   ${HiResInts}/2)
echo "lesewert TS_Counter_Wd1 =" $(ebRead  ${URL}   ${TS_Counter_Wd1}/2)
echo "lesewert TS_Counter_Wd2 =" $(ebRead  ${URL}   ${TS_Counter_Wd2}/2)
echo "lesewert TS_Counter_Wd3 =" $(ebRead  ${URL}   ${TS_Counter_Wd3}/2)
echo "lesewert TS_Counter_Wd4 =" $(ebRead  ${URL}   ${TS_Counter_Wd4}/2)
echo "lesewert TS_Cntr_Tag_LW =" $(ebRead  ${URL}   ${TS_Cntr_Tag_LW}/2)
echo "lesewert TS_Cntr_Tag_HW =" $(ebRead  ${URL}   ${TS_Cntr_Tag_HW}/2)
echo "----------------Remember: Fifo Words are read only--------------------"
ebWrite  ${URL}   ${PMFifoWords1}/2 $data
ebWrite  ${URL}   ${PMFifoWords2}/2 $data
ebWrite  ${URL}   ${PMFifoWords3}/2 $data
ebWrite  ${URL}   ${PMFifoWords4}/2 $data
echo "lesewert PMFifoWords1 =" $(ebRead  ${URL}   ${PMFifoWords1}/2)
echo "lesewert PMFifoWords2 =" $(ebRead  ${URL}   ${PMFifoWords2}/2)
echo "lesewert PMFifoWords3 =" $(ebRead  ${URL}   ${PMFifoWords3}/2)
echo "lesewert PMFifoWords4 =" $(ebRead  ${URL}   ${PMFifoWords4}/2)
echo "----------------Remember: Fifo Words are read only--------------------"
ebWrite  ${URL}   ${DaqFifoWords1}/2 $data
ebWrite  ${URL}   ${DaqFifoWords2}/2 $data
ebWrite  ${URL}   ${DaqFifoWords3}/2 $data
ebWrite  ${URL}   ${DaqFifoWords4}/2 $data
echo "lesewert DaqFifoWords1 =" $(ebRead  ${URL}   ${DaqFifoWords1}/2)
echo "lesewert DaqFifoWords2 =" $(ebRead  ${URL}   ${DaqFifoWords2}/2)
echo "lesewert DaqFifoWords3 =" $(ebRead  ${URL}   ${DaqFifoWords3}/2)
echo "lesewert DaqFifoWords4 =" $(ebRead  ${URL}   ${DaqFifoWords4}/2)
echo "----------------------------------------------------------------------"




# CtrlReg_Ch1 :Write SCU Slot Number to Bit 15..12 and Enable PostMortem with Bit 0 
ebWrite  ${URL}   ${CtrlReg_Ch1}/2 0x2001

ebWrite  ${URL}   ${TrigLW_Ch1}/2 0xcafe
ebWrite  ${URL}   ${TrigHW_Ch1}/2 0xbabe
ebWrite  ${URL}   ${TrigDly_Ch1}/2 0xdada


echo "Rücklesen CtrlReg_Ch1 (PM Bit 0 enabled) =" $(ebRead  ${URL}   ${CtrlReg_Ch1}/2)
sleep 1




ebWrite  ${URL}   ${CtrlReg_Ch1}/2 0x2000
echo "Rücklesen CtrlReg_Ch1 (PM Bit 0 disabled)=" $(ebRead  ${URL}   ${CtrlReg_Ch1}/2)
echo  "PMFifoWord zeigt zu Beginn" $(ebRead  ${URL}   ${PMFifoWords1}/2)


echo "x3FE=1022 Fifo Words, jetzt wird 1024x ausgelesen"


for i in {1..1030}
do 
  echo "PMDat_Ch1 #"$i"=" $(ebRead  ${URL}   ${PMDat_Ch1}/2) "remaining PMFifoWord" $(ebRead  ${URL}   ${PMFifoWords1}/2)

done
echo "Dont care on script errors now"
BEGINCOMMENT
ENDCOMMENT
echo "Script finished"

###############################################################################################
