#-------------------------------------------------------------------------------
# Name:
# Purpose:
#
# Author:      angel
#
# Created:     12.07.2016
# Copyright:   (c) angel 2016
# Licence:     <your licence>
#-------------------------------------------------------------------------------

encodings = {
    'UTF-8':      'utf-8',
    'CP1251':     'windows-1251',
}


def get_codepage(str = None):
    uppercase = 1
    lowercase = 3
    utfupper = 5
    utflower = 7
    codepages = {}
    for enc in encodings.keys():
        codepages[enc] = 0
    if str is not None and len(str) > 0:
        last_simb = 0
        for simb in str:
            simb_ord = ord(simb)

            """non-russian characters"""
            if simb_ord < 128 or simb_ord > 256:
                continue

            """UTF-8"""
            if last_simb == 208 and (143 < simb_ord < 176 or simb_ord == 129):
                codepages['UTF-8'] += (utfupper * 2)
            if (last_simb == 208 and (simb_ord == 145 or 175 < simb_ord < 192)) \
                or (last_simb == 209 and (127 < simb_ord < 144)):
                codepages['UTF-8'] += (utflower * 2)

            """CP1251"""
            if 223 < simb_ord < 256 or simb_ord == 184:
                codepages['CP1251'] += lowercase
            if 191 < simb_ord < 224 or simb_ord == 168:
                codepages['CP1251'] += uppercase

            last_simb = simb_ord

        idx = ''
        max = 0
        for item in codepages:
            if codepages[item] > max:
                max = codepages[item]
                idx = item
        return idx

def main():
    #sourceText = file("g:\\utf.txt").read()
    sourceText = raw_input()
    encoding = encodings[get_codepage(sourceText)]
    u = unicode(sourceText, encoding)
    idnaconverted = u.encode("idna")
    print idnaconverted

if __name__ == '__main__':
    main()
