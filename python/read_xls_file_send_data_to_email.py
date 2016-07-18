#-------------------------------------------------------------------------------
# Name:        bmko orders
# Purpose:
#
# Author:      Alexei Mikheev
#
# Created:     23.08.2012
# Copyright:   (c) Alexei Mikheev 2012
#-------------------------------------------------------------------------------
import os
import fnmatch
import time
import datetime
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.header import Header
import win32serviceutil

#настройки скрипта
file_settings = "settings.ini"
#путь файла настроек
pathsettings = "X:\\Tehnograd\\"


#путь до временной папки
tmppath = "c:\Temp"
#путь до файла заявок
orderpath = 'H:\\Заявки ИТ\\'

logfile = open(pathsettings + 'bmko.log', 'a')

class ExNonConnected(Exception): pass
def sendMail(str1):

    russian = 'windows-1251'
    EmailFrom = ""
    EmailTo = ["tg@tehgrad.ru"]

    msg = MIMEMultipart()
    msg["Subject"] = Header(str1, russian)
    msg["To"] = EmailTo1.encode('cp1251')
    text = MIMEText(str1.encode('cp1251'), 'plain', russian)
    msg.attach(text)
    txt = msg.as_string()
    ##ipserver = '10.0.1.251'
    ipserver = '192.168.4.250'

    server = smtplib.SMTP()
    try:
        server.connect(ipserver)
        server.sendmail(EmailFrom, EmailTo, msg.as_string())

    except Exception:
        x = ExNonConnected()
        curtime = (datetime.datetime.now()).__str__()
        logfile.write(curtime + '\n' + 'Mail server: ' + ipserver +' not found or not connected!\n')
        raise x

    finally:
        server.quit()
        del server
    return

def readfile(i, lwt):
    j = int(i,10)
    flag = False
    del i
    delta = j
    from win32com.client import Dispatch
    try:
        excel = Dispatch("Excel.Application")
        filexls = excel.Workbooks.Open(orderpath + order)
        sheet = filexls.Worksheets.Item(1)
        cur = sheet.Cells.Item(j,6)
        while cur.Text != "":
            cur = sheet.Cells.Item(j,6)
            flag = True
            j += 1

        if flag == True:
            j -= 1
            while delta < j:
                str1 = ""
                str1 = str1 + (sheet.Cells.Item(delta,3)).Text + " "
                str1 = str1 + (sheet.Cells.Item(delta,4)).Text + " "
                str1 = str1 + (sheet.Cells.Item(delta,5)).Text + " "
                str1 = str1 + (sheet.Cells.Item(delta,6)).Text + " "
                str1 = str1 + (sheet.Cells.Item(delta,7)).Text
                delta += 1
                sendMail(str1)
            fset = open(pathsettings + file_settings, 'w')
            alist = []
            alist += lwt
            str1 = str(j)
            alist += str1
            fset.writelines(alist)
            fset.close()
            curtime = (datetime.datetime.now()).__str__()
            logfile.write(curtime + '\n' + 'File Settings.ini changed succesfully!\n')
    except ExNonConnected:
            connected = False

    finally:
        del cur
        del sheet
        filexls.Close(SaveChanges = 0)
        del filexls
        excel.Workbooks.Close()
        excel.Quit()
        del excel
    return

	
try:
    order = None
    for file in os.listdir(orderpath):
        if fnmatch.fnmatch(file, 'Заявки*.xls'):
            order = file
            break
        else:
            curtime = (datetime.datetime.now()).__str__()
            logfile.write(curtime + '\n' + 'File Заявки.xls not found!\n')
            logfile.close()
            exit()
    xls = os.stat(orderpath + order)
except ValueError:
    curtime = (datetime.datetime.now()).__str__()
    logfile.write(curtime + '\n' + 'File Заявки.xls not found!\n')
    logfile.close()
    exit()

setfile = open(pathsettings + file_settings)
rows = setfile.readlines()
setfile.close()
d = datetime.datetime.fromtimestamp(xls[8])
xlstime = "" + d.__str__() + "\n"
try:
    if xlstime != rows[0]:
        readfile(rows[1], xlstime)
except IndexError:
    curtime = (datetime.datetime.now()).__str__()
    logfile.write(curtime + '\n' + 'File Settings.ini not found or empty\n')
logfile.close()

