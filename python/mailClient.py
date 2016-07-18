import imaplib, email
from email.parser import Parser
import os
import datetime
import time
import subprocess

from multiprocessing import Process

class MessageEntry:
	def __init__(self, ConvertedDate, fromAdress, Subject, DateStr = None):
		self.ConvertedDate = ConvertedDate
		self.DateStr = DateStr
		self.fromAdress = fromAdress
		self.subject = Subject

	def __repr__(self):
		return repr((self.ConvertedDate, self.DateStr, self.fromAdress, self.subject))

class MailClient:
	def __init__(self):
		self.server = None
		self.CurMessageIDS = []
		self.NewMessageIDS = []
		self.curDir =  os.path.abspath(os.curdir)
		self.fileName = 'messages.mci'

	def ReadMessagesFromBase(self):
		lFile = None
		try:
			lFile = open(self.curDir + '\\' + self.fileName)
		except IOError:
			lFile = None

		if lFile:
			line = lFile.readline()
			self.CurMessageIDS = line.split(';;;')
			lFile.close()

	def CheckIDMessage(self, searchedID):
		result = True
		if self.CurMessageIDS:
			try:
				msg = self.CurMessageIDS.index(searchedID)
			except ValueError:
				result = False
		else:
			result = False
		return result

	def AddNewID(self, newID):
		self.NewMessageIDS.append(newID)


	def WriteIndexes(self):
		if self.NewMessageIDS:
			f = open(self.curDir + '\\' + self.fileName, 'a')
			for msg in self.NewMessageIDS:
				self.CurMessageIDS.append(msg)
				f.write(msg + ';;;')
			f.close()

	def ClearBufferNewestMessages(self):
		self.NewMessageIDS = []



def LoadMessagesFromServer(client):
	try:
		M = imaplib.IMAP4('192.168.4.250')
		M.login('tg@tehgrad.ru', '12345')
		M.select('INBOX')
		
	except Exception:
		curDate = datetime.datetime.now()
		curDateStr = curDate.strftime("%d/%m/%Y %H:%M:%S")
		print(curDateStr + ' No connected to mail server!')
		return None
	typ, data = M.search(None, 'ALL')
	numbers = data[0].split()
	MessagesListToSend = []
	LastNum = len(numbers)
	numCheckedMess = 5
	isNotMessagesToBase = True
	while isNotMessagesToBase:
		typ, data = M.fetch(LastNum, '(RFC822.HEADER)')
		msg = data[0][1]
		headers = Parser().parsestr(msg)
		curID = headers['Message-ID']
		if client.CheckIDMessage(curID):
			numCheckedMess -= 1
		else:
			FromStr = headers['From']
			subjectStr = headers['Subject']
			if not subjectStr:
				subjectStr = "Empty subject"
			DateStr = headers['Date']
			isConverted, ConvertedDate = FoundDate(DateStr)
			if isConverted:
				newMsg = MessageEntry(ConvertedDate, FromStr, subjectStr)
			else:
				newMsg = MessageEntry(ConvertedDate, FromStr, subjectStr, DateStr)
			MessagesListToSend.append(newMsg)
			client.AddNewID(curID)
		LastNum -= 1

		if LastNum and numCheckedMess:
			continue
		else:
			isNotMessagesToBase = False

	M.close()
	M.logout()
	if MessagesListToSend:
		MessagesListToSend = sorted(MessagesListToSend, key=lambda MessageEntry: MessageEntry.ConvertedDate)
	return MessagesListToSend

def FoundDate(dateStr):
	formatDate = 1
	result = False
	ConvertedDate = None
	isConvertedDate = True
	pos = -1
	while not result:
		if formatDate == 1:
			pos = dateStr.find('(')
			if pos > 0:
				try:
					buff = dateStr[:pos - 7]
					ConvertedDate = datetime.datetime.strptime(buff, "%a, %d %b %Y %H:%M:%S")
					result = True
				except ValueError as e:
					ConvertedDate = None
					formatDate = 2
					continue
			else:
				formatDate = 3
		elif formatDate == 2:
			pos = dateStr.find('(')
			if pos > 0:
				try:
					buff = dateStr[:pos - 7]
					ConvertedDate = datetime.datetime.strptime(buff, "%d %b %Y %H:%M:%S")
					result = True
				except ValueError as e:
					ConvertedDate = None
					formatDate += 1
					continue
			else:
				formatDate = 3

		elif formatDate == 3:
			pos = dateStr.find('GMT')
			if pos > 0:
				try:
					buff = dateStr[:pos - 1]
					ConvertedDate = datetime.datetime.strptime(buff, "%a, %d %b %Y %H:%M:%S")
					result = True
				except ValueError:
					ConvertedDate = None
					formatDate += 1
					continue
			else:
				formatDate = 5

		elif formatDate == 4:
			pos = dateStr.find('GMT')
			if pos > 0:
				try:
					buff = dateStr[:pos - 1]
					ConvertedDate = datetime.datetime.strptime(buff, "%d %b %Y %H:%M:%S")
					result = True
				except ValueError:
					ConvertedDate = None
					formatDate += 1

		elif formatDate == 5:
				buff = dateStr[:pos-5]
				try:
					ConvertedDate = datetime.datetime.strptime(buff, "%a, %d %b %Y %H:%M:%S")
					result = True
				except ValueError:
					ConvertedDate = None
					formatDate = 6
					continue
		elif formatDate == 6:
				buff = dateStr[:pos-5]
				try:
					ConvertedDate = datetime.datetime.strptime(buff, "%d %b %Y %H:%M:%S")
					result = True
				except ValueError:
					ConvertedDate = None
					formatDate += 1
		else:
			ConvertedDate = datetime.datetime.now()
			isConvertedDate = False
			result = True

	return isConvertedDate, ConvertedDate

def StartupHelp():
	message = "Mail Client with ICQ support (2013).\nVersion 1.0.1\nCopyright Alexey Mikheev 2011-2013.\n\n"
	print(message)


def SendToICQ(messages):
	result = False
	for msg in messages:
		if not result:
			result = True
		if not msg.DateStr:
			resultDateStr = msg.ConvertedDate.strftime("%d.%m.%Y %H:%M:%S")
		else:
			resultDateStr = msg.DateStr
		helperStr = resultDateStr + ";;;" + msg.fromAdress + ";;;" + msg.subject
		subprocess.call(['ICQHelper.exe', helperStr])
	return result


def Worker():
	StartupHelp()
	mc = MailClient()
	mc.ReadMessagesFromBase()
	print('Waiting messages...')
	while True:
		#print('Check messages...')
		Messages = LoadMessagesFromServer(mc)
		if Messages:
			count = len(Messages)
			#print('Send messages...')
			if SendToICQ(Messages):
				curDate = datetime.datetime.now()
				curDateStr = curDate.strftime("%d/%m/%Y %H:%M:%S")
				print(curDateStr + ' Sended ' + str(count) + ' messages.')
				mc.WriteIndexes()
			else:
				print('No messages to send!\n')
		#print('Sleeping... 60 sec.\n')
		time.sleep(60)

def main():
	while True:
		w = Process(target=Worker)
		w.start()
		w.join()
	return

if __name__ == '__main__':
    main()
