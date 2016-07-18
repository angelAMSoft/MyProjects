import os, sys
import fnmatch
from stat import *
#projectClass = "Window"
projectClass = "Console"
projectname = "ZBAR"
sourcedir = "H:\\dev_projects\\QRCode\\ZBARProj\\LIBZBAR\\src"
projfile = open(sourcedir + "\\" + 'TemplateConsole.dsp', 'a')

def walktree(top, callback):
    '''recursively descend the directory tree rooted at top,
       calling the callback function for each regular file'''
    for f in os.listdir(top):

        pathname = os.path.join(top, f)
        mode = os.stat(pathname).st_mode
        if S_ISDIR(mode):
            printString("# Begin Group \"" + f + "\"\n")
            printString("# PROP Default_Filter \"\"")
            walktree(pathname, callback)
            printString("# End Group")
        elif S_ISREG(mode):
            # It's a file, call the callback function
            callback(pathname)
        else:
            # Unknown file type, print a message
            print('Skipping %s' % pathname)
def printString(newstring):
    projfile.write(newstring + '\n')
    ##print(newstring + '\n')

def visitfile(file):
    sourcefile = None
    headerfile = None
    if fnmatch.fnmatch(file, '*.c'):
        sourcefile = file
    elif fnmatch.fnmatch(file, '*.cpp'):
        sourcefile = file
    elif fnmatch.fnmatch(file, '*.cxx'):
        sourcefile = file
    else:
        if fnmatch.fnmatch(file, '*.h'):
            headerfile = file
        elif fnmatch.fnmatch(file, '*.hxx'):
            headerfile = file
        elif fnmatch.fnmatch(file, '*.hpp'):
            headerfile = file
    if sourcefile is not None:
        ##print('visiting source file', file)
        printString("# Begin Source File" + '\n')
        printString("SOURCE=\"" + file + "\"")
        printString("# End Source File")
    if headerfile is not None:
        ##print('visiting header file', file)
        printString("# Begin Source File" + '\n')
        printString("SOURCE=\"" + file + "\"")
        printString("# End Source File")

if __name__ == '__main__':
    printString("# Begin Group " + "\"" + projectname + "\"\n")
    printString("# PROP Default_Filter \"\"")
    walktree(sourcedir, visitfile)
    printString("# End Group")
    printString("# End Target")
    printString("# End Project")
    projfile.close()

