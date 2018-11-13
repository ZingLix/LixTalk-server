import socket
import json,threading
import time

def soc_thread(soc,i):
    while (flag==False):
        time.sleep(0.1)
    t1=time.time()
    data = {
        'recver_id':0,
        'type' : 1,
        'username' : str(i),
        'password' : str(i)
    }
    soc.send(json.dumps(data).encode())
    soc.recv(1024)
    t2=time.time()
    print((t2-t1).se)

flag=False
host = '118.25.27.241'
port =9981
test_size=1000


socketlist=[]
for i in range(0,test_size):
    tmp=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tmp.connect((host,port))
    socketlist.append(tmp)

i=0
for s in socketlist:
    t=threading.Thread(target=soc_thread,name='socThread',args=(s,i))
    t.start()
    t.join()
    i=i+1
flag=True

#s.send("asdfasdf".encode())