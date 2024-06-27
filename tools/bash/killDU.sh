#!/bin/bash
killall -9 $(basename $(ps | grep _DU | grep -v grep | awk '{print $4}'))
