import os
import datetime
import fnmatch
import time
import ctypes
pathToLogs = "C:\\Windows\\System32\\winevt\\Logs\\"

#30 gb
TargetFreeSpace = 32212254720

class LogEntry:
	def __init__(self, LogDate, LogName):
		self.LogDate = LogDate
		self.LogName = LogName

	def __repr__(self):
		return repr((self.LogDate, self.LogName))

class LogFile:
	def __init__(self, path):
		self.path = path
		self.LogName = 'ClearLogs.log'

	def WriteToLog(self, message):
		curDate = datetime.datetime.now()
		curDateStr = curDate.strftime("%d/%m/%Y %H:%M:%S")
		logMessage = curDateStr + ' ' + message + '\n'
		f = open(self.path + '\\' + self.LogName, 'a')
		f.write(logMessage)
		f.close()

#Archive-Security-2013-11-12-19-00-39-433.evtx
class ClearLog:
	def __init__(self):
		self.daysLogs = []
		self.findFiles = False
		self.FreeSpace = 0
		self.currentIndex = 0
		self.curDir = os.path.abspath(os.curdir)
		self.log = LogFile(self.curDir)

	def FindCurrentLogs(self):
		AllCurrentSecLogs = []
		PartFind = 'Security'
		findFiles = False
		ListLogs = None
		try:
			ListLogs = os.listdir(pathToLogs)
		except WindowsError as e:
			message = 'Obtain Directory: ' + pathToLogs + ' Failed with Windows Error -  ' + str(e.winerror) + '!'
			self.log.WriteToLog(message)
		for curFile in ListLogs:
			if fnmatch.fnmatch(curFile, "Archive-Security*.evtx"):
				index = curFile.find(PartFind)
				indexExt = curFile.find(".evtx")
				strDate = curFile[index + len(PartFind) + 1:indexExt]
				createDate = datetime.datetime.strptime(strDate, "%Y-%m-%d-%H-%M-%S-%f")
				curLogFile = LogEntry(createDate, curFile)
				AllCurrentSecLogs.append(curLogFile)
				findFiles = True

		if findFiles:
			DayLogs = []
			curDateLog = None
			firstEntry = True
			NextDate = False
			AllCurrentSecLogs = sorted(AllCurrentSecLogs, key=lambda LogEntry: LogEntry.LogDate)
			for curEntry in AllCurrentSecLogs:
				if firstEntry:
					curDateLog = curEntry.LogDate.date()
					firstEntry = False

				comparableDate = curEntry.LogDate.date()
				if comparableDate == curDateLog:
					DayLogs.append(curEntry)
				elif comparableDate > curDateLog:
					self.daysLogs.append(DayLogs)
					curDateLog = curEntry.LogDate.date()
					DayLogs = []
					DayLogs.append(curEntry)

			if DayLogs:
				self.daysLogs.append(DayLogs)

	def CurrentFreeSpace(self):
		free_bytes = ctypes.c_ulonglong(0)
		total_bytes = ctypes.c_ulonglong(0)
		driveLetter = pathToLogs[0]
		driveLetter +=":\\"
		ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(driveLetter), None, ctypes.pointer(total_bytes), ctypes.pointer(free_bytes))
		self.FreeSpace = free_bytes.value

	def DeleteLogs(self):
		self.CurrentFreeSpace()
		if self.FreeSpace < TargetFreeSpace:
			NeedDelete = True
		if NeedDelete and self.daysLogs:
			isDeletedDay = False
			LenghtEntry = len(self.daysLogs)
			while NeedDelete == True and self.FreeSpace < TargetFreeSpace:
				DeletedDay = self.daysLogs[self.currentIndex]
				for delEntry in DeletedDay:
					try:
						os.remove(pathToLogs + delEntry.LogName)
						print('Remove file: ' + delEntry.LogName)
					except IOError:
						message = 'Remove file: ' + delEntry.LogName + ' Failed!'
						self.log.WriteToLog(message)
					except WindowsError as e:
						#WindowsError.message
						message = 'Remove file: ' + delEntry.LogName + ' Failed with Windows Error -  ' + str(e.winerror) + '!'
						self.log.WriteToLog(message)

				self.currentIndex = self.currentIndex + 1
				self.CurrentFreeSpace()
				if self.currentIndex >= LenghtEntry:
					NeedDelete = False



def main():
	cl = ClearLog()
	cl.FindCurrentLogs()
	cl.DeleteLogs()

if __name__ == '__main__':
    main()
