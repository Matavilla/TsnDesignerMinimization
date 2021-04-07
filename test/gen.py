import random
import os
import sys
import subprocess
import argparse

NumTests = 1
NumTestsWith100 = 0
TotalLen = 0
TotalNumMsg = 0
TotalBaseLen = 0
TotalBaseNumMsg = 0
MaxLen = 0

NumMsg = 10
MsgSizeMin = [1500, 16]
MsgSizeMax = [100000, 1500]
PeriodMin = [100, 10]
PeriodMax = [10000, 1000]
TMaxMin = [10, 10]
TMaxMax = [1000, 100]
DestNumMin = 1
DestNumMax = 1

def createParser():
    parser = argparse.ArgumentParser(description="generate tests for topology")
    parser.add_argument('--dirName', type=str)
    parser.add_argument('--fileName', type=str)
    parser.add_argument('--numES', type=str)
    parser.add_argument('--baseTest', type=bool)
    parser.add_argument('--extraLen', type=int)
    parser.add_argument('--typeClass', type=int)
    parser.add_argument('--numMsg', type=int, default=10)
    parser.add_argument('--numTests', type=int, default=1)
    return parser

def checkAns(baseTest, out):
    global MaxLen
    print(out)
    s = out.split("########################################")
    l = s[0].split('\n')
    m = s[2].split('\n')
    tmp1 = int(l[0].split()[1])
    tmp2 = int(m[0].split()[1])
    MaxLen = int(l[1].split()[1])
    if baseTest:
        global TotalBaseLen, TotalBaseNumMsg
        TotalBaseLen += tmp1
        TotalBaseNumMsg += tmp2
    else:
        global TotalLen, TotalNumMsg
        TotalLen += tmp1
        TotalNumMsg += tmp2
    return (tmp1, tmp2)

def printAns(namespace):
    if namespace.baseTest:
        print("for base topology:")
        print(f"     TotalLen: {TotalBaseLen / NumTests}")
        print(f"     TotalMsg: {TotalBaseNumMsg / NumTests}")
    print("for full topology:")
    print(f"     TotalLen: {TotalLen / NumTests}")
    print(f"     %Len: {(TotalLen / NumTests) / MaxLen}")
    print("\n")
    print(f"     TotalMsg: {TotalBaseNumMsg / NumTests}")
    print(f"     %Msg: {(TotalBaseNumMsg / NumTests) / NumMsg}")
    print("\n")
    print(f"     NumTestsWith100: {NumTestsWith100 / NumTests}")
    print(f"     %Extr: {(MaxLen - TotalLen / NumTests) / namespace.extraLen}")

def generateOneTest(fileName, type, data):
    text = ""
    f = open(f"{fileName}_{type}.xml", 'r')
    textArr = f.readlines()
    text = "".join(textArr)

    for it in data.ES:
        text += "<ES>" + os.linesep
        for sender in it[0]:
            text += "<SenderMSG>" + str(sender) + "</SenderMSG>" + os.linesep
        for receiver in it[1]:
            text += "<ReceiverMSG>" + str(receiver) + "</ReceiverMSG>" + os.linesep
        text += "</ES>" + os.linesep

    text += "</Network>" + os.linesep

    for it in data.MSG:
        text += "<Msg>"
        text += "<Type>" + it[0] + "</Type>"
        text += "<T>" + str(it[1]) + "</T>"
        text += "<Size>" + str(it[2]) + "</Size>"
        text += "<MaxDur>" + str(it[3]) + "</MaxDur>"
        text += "</Msg>" + os.linesep
    return text

class Data:
    ES = []
    MSG = []

def generateData(namespace):
    data = Data()
    t = 0
    if namespace.typeClass == 3:
        t = random.randint(0, 100)
        if (t < 75):
            t = 0
        else:
            t = 1
    elif namespace.typeClass == 2:
        t = 1

    return data

parser = createParser()
namespace = parser.parse_args(sys.argv[1:])
NumMsg = namespace.numMsg
NumTests = namespace.numTests

if not os.path.exists(namespace.dirName):
    os.makedirs(namespace.dirName)

os.system(f"rm -rf {namespace.dirName}/*")

for i in range(1, NumTests + 1):
    flag = True
    while flag:
        fileNameForFull = f"{namespace.dirName}/test_{i}_full.xml"
        fileNameForBase = f"{namespace.dirName}/test_{i}_base.xml"

        data = generateData(namespace)

        if namespace.baseTest:
            f = open(fileNameForBase, 'w')
            f.write(generateOneTest(namespace.fileName, "Base", data))
            f.close()

        f = open(fileNameForFull, 'w')
        f.write(generateOneTest(namespace.fileName, "Full", data))
        f.close()

        flag = False
        if namespace.baseTest:
            process = subprocess.run(f"./TsnDesigner --dataPath {fileNameForBase}", stdout=subprocess.PIPE)
            length, numMsg = checkAns(namespace.baseTest, process.stdout.decode('utf-8'))

            if numMsg != NumMsg:
                flag = True
                continue

        process = subprocess.run(f"./TsnDesigner --dataPath {fileNameForFull}", stdout=subprocess.PIPE)
        length, numMsg = checkAns(False, process.stdout.decode('utf-8'))
        if numMsg == NumMsg:
            NumTestsWith100 += 1
printAns(namespace)
