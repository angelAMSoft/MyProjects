import os
import datetime
import fnmatch
import time
import zipfile
import ctypes


pathToBackup = "X:\\backup\\"
#compress 4 Gb
extraBytesCompress = 4294967296
#uncompress 12 Gb
extraBytesUncompress = 12884901888

def CurrentFreeSpace(path):
    free_bytes = ctypes.c_ulonglong(0)
    total_bytes = ctypes.c_ulonglong(0)
    driveLetter = path[0]
    driveLetter +=":\\"
    ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(driveLetter), None, ctypes.pointer(total_bytes), ctypes.pointer(free_bytes))

    return free_bytes.value

# Last:true - find last backup
# Last:false - file older backup
def FindBackup(Last):
	backupStatFiles = []
	findFiles = False
	for Curfile in os.listdir(pathToBackup):
		if fnmatch.fnmatch(Curfile, '*.zip'):
			index = Curfile.find("_")
			indexExt = Curfile.find(".zip")
			strDate = Curfile[index + 1:indexExt]
			lastmod_date = datetime.datetime.strptime(strDate, "%Y-%m-%d")
			curStat = lastmod_date, Curfile
			backupStatFiles.append(curStat)
			findFiles = True

	lastFile = None
	if not findFiles:
		return None
	backupStatFiles.sort()
	if Last:
    		lastFile = max(backupStatFiles)
	else:
            lastFile = min(backupStatFiles)

	return lastFile

def DeleteOlderArhive():
	oldFile = FindBackup(False)
	res = False
	if oldFile != None:
		os.remove(pathToBackup + oldFile[1])
	else:
		res = True
	return res

def FreeSpaceToNewArhive(lastfile):
    fileName = lastfile[1]
    zf = zipfile.ZipFile(pathToBackup + fileName)
    info = zf.infolist()
    fileSize = 0
    fileSize += info[0].compress_size
    fileSize += info[0].file_size
    fileSize += extraBytesCompress + extraBytesUncompress
    zf.close()
    return fileSize

lastFile = FindBackup(True)
if lastFile is None:
	exit()
NewFileSize = FreeSpaceToNewArhive(lastFile)
freespace = CurrentFreeSpace(pathToBackup)
NoFiles = False
while NoFiles == False and freespace < NewFileSize:
    NoFiles = DeleteOlderArhive()
    freespace = CurrentFreeSpace(pathToBackup)
