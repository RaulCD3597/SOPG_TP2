import socket
import sys
import time
import thread
import datetime
import os

def checkFiles():
        if not os.path.exists('/tmp/out0.txt'):
                fp = open("/tmp/out0.txt","w+")
                fp.write("0")
                fp.close()
        if not os.path.exists('/tmp/out1.txt'):
                fp = open("/tmp/out1.txt","w+")
                fp.write("0")
                fp.close()
        if not os.path.exists('/tmp/out2.txt'):
                fp = open("/tmp/out2.txt","w+")
                fp.write("0")
                fp.close()
        if not os.path.exists('/tmp/out3.txt'):
                fp = open("/tmp/out3.txt","w+")
                fp.write("0")
                fp.close()


def writeOutState(outNumber,state):
        fp = open("/tmp/out"+str(outNumber)+".txt","w+")
        fp.write(state)
        fp.close()
        return True

def readOutState(outNumber):
        fp = open("/tmp/out"+str(outNumber)+".txt","r")
        val = fp.read()
        fp.close()
        return val[0]


def toggleLineState(lineNumber):
        st = readOutState(lineNumber)
        if st=="0":
                print("Nuevo estado de linea:"+str(lineNumber)+" es:1")
                writeOutState(int(lineNumber),"1")
        elif st=="1":
                print("Nuevo estado de linea:"+str(lineNumber)+" es:2")
                writeOutState(int(lineNumber),"2")
        else:
                print("Nuevo estado de linea:"+str(lineNumber)+" es:0")
                writeOutState(int(lineNumber),"0")

def sendLinesStates(sock):
        o0 = readOutState(0)
        o1 = readOutState(1)
        o2 = readOutState(2)
        o3 = readOutState(3)
        p = ":STATES"+o0+o1+o2+o3+"\n"
        print("envio '"+p+"'")
        sock.send(p)


def rcvThread(sock):
        global socketOk
        global outChanged
        print("INICIO thread recepcion")
        while True:
                data = sock.recv(128)
                if len(data)==0:
                        print("Se cerro la conexion")
                        break
                print("LLEGO:"+data)
                try:
                        data = data.split(":LINE")
                        data = data[1].split("TG\n")
                        print(data[0])
                        lineNumber = int(data[0])
                        toggleLineState(lineNumber)
                        outChanged=True
                except:
                        print("error enla trama")

        print("FIN thread recepcion")
        socketOk=False

socketOk=True
outChanged=True

while True:
        try:
                # Creo TCP/IP socket
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                server_address = ('127.0.0.1', 10000)
                print >>sys.stderr, 'connecting to %s port %s' % server_address
                sock.connect(server_address)
                socketOk=True
                # Creo thread para escuchar paquetes
                thread.start_new_thread( rcvThread, (sock, ) )

                #creo archivos si no existen
                checkFiles()

                fc=[-1,-1,-1,-1]

                while socketOk:
                        time.sleep(0.5)

                        if readOutState(0)!=fc[0]:
                                fc[0] = readOutState(0)
                                outChanged=True
                        if readOutState(1)!=fc[1]:
                                fc[1] = readOutState(1)
                                outChanged=True
                        if readOutState(2)!=fc[2]:
                                fc[2] = readOutState(2)
                                outChanged=True
                        if readOutState(3)!=fc[3]:
                                fc[3] = readOutState(3)
                                outChanged=True


                        if outChanged:
                                outChanged=False
                                sendLinesStates(sock)

                raise Exception

        except:			
                time.sleep(1)
                print("Socket invalido, reintento...")
