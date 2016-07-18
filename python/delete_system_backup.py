#-------------------------------------------------------------------------------
# Name: System backup delete utility
# Purpose:
#
# Author:      Alexei Mikheev
#
# Created:     16.10.2014
# Copyright:   (c) Alexei Mikheev 2014
#-------------------------------------------------------------------------------

import os
import datetime
import fnmatch
import time
import ctypes
import subprocess

#50gb
TargetFreeSpace = 1073741824 * 50
pathToBackup = "E:\\System Volume Information\\"
pathToScript = "D:\\admin\\\DeleteArhive\\script.txt"
pathToLog = "D:\\admin\\\DeleteArhive\\"


def WriteToLog(message):
	curDate = datetime.datetime.now()
	curDateStr = curDate.strftime("%d/%m/%Y %H:%M:%S")
	logMessage = curDateStr + ' ' + message + '\n'
	f = open(pathToLog + 'Deleted_Arhive.txt', 'a')
	f.write(logMessage)
	f.close()


def DeleteOlderArhive():
	subprocess.call(["WrapperDiskShadow.exe", pathToScript])
	return

def CurrentFreeSpace(path):
    free_bytes = ctypes.c_ulonglong(0)
    total_bytes = ctypes.c_ulonglong(0)
    driveLetter = path[0]
    driveLetter +=":\\"
    ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(driveLetter),
												None, ctypes.pointer(total_bytes),
												ctypes.pointer(free_bytes))

    return free_bytes.value

def main():
	NumDeletedBackups = 0
	OldFreeSpace = None
	freespace = CurrentFreeSpace(pathToBackup)
	while freespace < TargetFreeSpace:
		if OldFreeSpace is None:
			OldFreeSpace = freespace
		NumDeletedBackups = NumDeletedBackups + 1
		DeleteOlderArhive()
		freespace = CurrentFreeSpace(pathToBackup)



	if NumDeletedBackups != 0:
		spaceStr = str((TargetFreeSpace - OldFreeSpace) / 1073741824)
		message = "Deleted " + NumDeletedBackups + " backups. Released " + spaceStr + " GB."
		WriteToLog(message)

if __name__ == '__main__':
	main()

