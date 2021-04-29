import random
import os
import sys
import subprocess
import argparse

NumTests = 1
NumTestsWith100 = 0
NumES = 0
TotalLen = 0
TotalNumMsg = 0
TotalBaseLen = 0
TotalBaseNumMsg = 0
MaxLen = 0

NumMsg = 10
MsgSizeMin = [1500, 16]
MsgSizeMax = [100000, 1500]
Periods = [[100, 120, 125, 150, 200, 240, 250, 300, 375, 400, 500, 600, 750, 1000, 1200, 1500, 2000, 3000, 6000], [15, 20, 48, 80, 100, 120, 125, 150, 200, 240, 250, 300, 375, 400, 500, 600, 750, 1000]]
TMaxMin = [10, 10]
TMaxMax = [1000, 100]
TypeMsg = ["TT", "A", "B"]
DestNumMin = 1
DestNumMax = 1

def createParser():
    parser = argparse.ArgumentParser(description="generate tests for topology")
    parser.add_argument('--dirName', type=str)
    parser.add_argument('--fileName', type=str)
    parser.add_argument('--numES', type=int)
    parser.add_argument('--baseTest', type=str)
    parser.add_argument('--extraLen', type=int)
    parser.add_argument('--typeClass', type=int)
    parser.add_argument('--numMsg', type=int, default=10)
    parser.add_argument('--numTests', type=int, default=1)
    return parser

def checkAns(out):
    global MaxLen
    #print(out)
    s = out.split("########################################")
    l = s[0].split('\n')
    m = s[2].split('\n')
    tmp1 = int(l[0].split()[1])
    tmp2 = int(m[1].split()[1])
    MaxLen = int(l[1].split()[1])
    return (tmp1, tmp2)

def printAns(namespace):
    if namespace.baseTest == "True":
        print("for base topology:")
        print(f"     TotalLen: {TotalBaseLen / NumTests}")
        print(f"     TotalMsg: {TotalBaseNumMsg / NumTests}")
        print("")
    print("for full topology:")
    print(f"     TotalLen: {round(TotalLen / NumTests, 2)}      TotalMsg: {round(TotalNumMsg / NumTests, 2)}      NumTestsWith100: {round(NumTestsWith100 / NumTests, 2)}")
    print(f"     %Len: {round((TotalLen / NumTests) / MaxLen, 2)}           %Msg: {round((TotalNumMsg / NumTests) / NumMsg, 2)}           %Extr: {round((MaxLen - TotalLen / NumTests) / namespace.extraLen, 2)}")

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
    data.ES.clear()
    data.MSG.clear()
    for i in range(NumES):
        data.ES.append([[], []])
    
    for i in range(NumMsg):
        t = 0
        if namespace.typeClass == 3:
            t = random.randint(0, 100)
            if (t < 75):
                t = 0
            else:
                t = 1
        elif namespace.typeClass == 2:
            t = 1
        sender = random.randint(0, NumES - 1)
        data.ES[sender][0].append(i + 1)

        for j in range(DestNumMin, DestNumMax + 1):
            rec = random.randint(0, NumES - 1)
            while rec == sender:
                rec = random.randint(0, NumES - 1)
            data.ES[rec][1].append(i + 1)
        
        periodGCL = 6000
        t2 = random.randint(0, 100)
        if (t2 < 50):
            t2 = 0
        elif (60 <= t2 <= 85):
            t2 = 1
        else:
            t2 = 2
        msgType = TypeMsg[t2]
        msgSize = random.randint(MsgSizeMin[t], MsgSizeMax[t])
        period = random.choice(Periods[t])
        time = msgSize / 125000.0
        while (time / period > 0.05):
            period = random.choice(Periods[t])
            time = msgSize / 125000.0
        tmp = (msgSize - MsgSizeMin[t] + 1) / MsgSizeMax[t]
        dur = TMaxMin[t] + int((TMaxMax[t] - TMaxMin[t]) * tmp)
        data.MSG.append([msgType, period, msgSize, dur])
    return data

parser = createParser()
namespace = parser.parse_args(sys.argv[1:])
NumMsg = namespace.numMsg
NumTests = namespace.numTests
NumES = namespace.numES

if not os.path.exists(namespace.dirName):
    os.makedirs(namespace.dirName)

os.system(f"rm -rf {namespace.dirName}/*")

for i in range(1, NumTests + 1):
    flag = True
    while flag:
        fileNameForFull = f"{namespace.dirName}/test_{i}_full.xml"
        fileNameForBase = f"{namespace.dirName}/test_{i}_base.xml"

        data = generateData(namespace)

        if namespace.baseTest == "True":
            f = open(fileNameForBase, 'w')
            f.write(generateOneTest(namespace.fileName, "Base", data))
            f.close()

        f = open(fileNameForFull, 'w')
        f.write(generateOneTest(namespace.fileName, "Full", data))
        f.close()

        flag = False
        if namespace.baseTest == "True":
            process = subprocess.run(f"./TsnDesigner --dataPath {fileNameForBase}", shell=True, stdout=subprocess.PIPE)
            length, numMsg = checkAns(process.stdout.decode('utf-8'))

            if numMsg != NumMsg:
                flag = True
                continue

            TotalBaseLen += length
            TotalBaseNumMsg += numMsg

        process = subprocess.run(f"./TsnDesigner --dataPath {fileNameForFull}", shell=True, stdout=subprocess.PIPE)
        length, numMsg = checkAns(process.stdout.decode('utf-8'))
        if numMsg == NumMsg:
            NumTestsWith100 += 1
        TotalLen += length
        TotalNumMsg += numMsg
        printAns(namespace)
print("##############################")
printAns(namespace)
