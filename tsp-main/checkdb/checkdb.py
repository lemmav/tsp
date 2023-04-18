import sqlite3, datetime
import time, requests, json
from kubernetes import client,config,utils

with open('init.json', 'r', encoding='utf-8') as fh: #открываем файл на чтение
    algorithms = json.load(fh) #загружаем из файла данные в словарь data
podmanager = "http://podmanager-service:8001"    
# def fromQueueToProgress():
#     sqlite_connection = None
#     try:
#         #поменялся адрес бд
#         sqlite_connection= sqlite3.connect('/checkdb/createdbs.sqlite', timeout=20)
#         # sqlite_connection= sqlite3.connect('/dbdir/createdbs.sqlite', timeout=20)
#         sqlite_connection.row_factory = sqlite3.Row
#         cursor = sqlite_connection.cursor()
#         print("fromQueueToProgress Подключен к SQLite")
#         for algorithm in algorithms.keys():
#             cursor.execute("SELECT * FROM progress WHERE algorithm=?", (algorithm,))
#             progressrow = cursor.fetchone()
#             print("SELECTED FROM progress")
#             if progressrow is None:
#                 cursor.execute("SELECT * FROM queue WHERE algorithm=? ORDER BY requesttime", (algorithm,))
#                 print("SELECTED FROM queue")
#                 queuerow = cursor.fetchone()
#                 if queuerow is not None:
#                     print(queuerow['graph'])
#                     cursor.execute("INSERT INTO progress (url, graph, algorithm, starttime) VALUES (?, ?, ?, datetime('now', 'localtime')) ", (queuerow['url'], queuerow['graph'], queuerow['algorithm']))
#                     cursor.execute("DELETE FROM queue WHERE url=?", (queuerow['url'],))
#                     print("INSERTED INTO progress and DELETED FROM queue")
#                     sqlite_connection.commit()
#                     # в podmanager
#                     # requests.post(algorithms[queuerow['algorithm']]+"/graph", data=queuerow['graph'])
#                     params={"url": queuerow['url'], "algorithm":queuerow['algorithm']}
#                     requests.post(":8001", params=params, data=queuerow['graph'])

#         cursor.close()
def fromQueueToProgress():
    sqlite_connection = None
    try:
        #поменялся адрес бд
        sqlite_connection= sqlite3.connect('/checkdb/createdbs.sqlite', timeout=20)
        # sqlite_connection= sqlite3.connect('/dbdir/createdbs.sqlite', timeout=20)
        sqlite_connection.row_factory = sqlite3.Row
        cursor = sqlite_connection.cursor()
        print("fromQueueToProgress Подключен к SQLite")
        for algorithm in algorithms.keys():          
            cursor.execute("SELECT * FROM queue WHERE algorithm=? ORDER BY requesttime", (algorithm,))
            print("SELECTED FROM queue")
            queuerow = cursor.fetchone()
            if queuerow is not None:
                print(queuerow['graph'])
                params={"url": queuerow['url'], "algorithm":queuerow['algorithm']}
                r = requests.post(podmanager, params=params, data=queuerow['graph'])
                if r.text == '1':
                    cursor.execute("INSERT INTO progress (url, graph, algorithm, starttime) VALUES (?, ?, ?, datetime('now', 'localtime')) ", (queuerow['url'], queuerow['graph'], queuerow['algorithm']))
                    cursor.execute("DELETE FROM queue WHERE url=?", (queuerow['url'],))
                    print("INSERTED INTO progress and DELETED FROM queue")
                    sqlite_connection.commit()
                    # в podmanager
                    # requests.post(algorithms[queuerow['algorithm']]+"/graph", data=queuerow['graph'])
  

        cursor.close()

    except sqlite3.Error as error:
        print("fromQueueToProgress Ошибка при подключении к sqlite", error)
    finally:
        if (sqlite_connection):
            sqlite_connection.close()
            print("Соединение с SQLite закрыто")

def doneCheck():
    try:
        sqlite_connection= sqlite3.connect('/checkdb/createdbs.sqlite', timeout=20)
        # sqlite_connection= sqlite3.connect('/dbdir/createdbs.sqlite', timeout=20)
        sqlite_connection.row_factory = sqlite3.Row
        cursor = sqlite_connection.cursor()
        print("doneCheck Подключен к SQLite")
        for algorithm in algorithms.keys():
            cursor.execute("SELECT * FROM progress WHERE algorithm=?", (algorithm,))
            progressrow = cursor.fetchone()
            print("SELECTED FROM progress")
            if progressrow is not None:
                # в podmanager
                params={"url": progressrow['url'], "algorithm":progressrow['algorithm']}
                progressrequest = requests.get(podmanager+'/progress',params=params)
                
                if int(float(progressrequest.text))==1:
                    # в podmanager
                    resultrequest = requests.get(podmanager+'/result', params=params)
                    resultrequest_str = resultrequest.text
                    resultrequest_str = resultrequest_str.split('\n')
                    print(resultrequest_str[-1])
                    cursor.execute("INSERT INTO result (url, answer, way, algorithm, executiontime) VALUES (?, ?, ?, ?, datetime('now', 'localtime')) ", (progressrow['url'], resultrequest_str[-1], resultrequest_str[1], progressrow['algorithm']))
                    cursor.execute("DELETE FROM progress WHERE url=?", (progressrow['url'],))
                    print("INSERTED INTO result and DELETED FROM progress")
                    sqlite_connection.commit()
                

        cursor.close()

    except sqlite3.Error as error:
        print("doneCheck Ошибка при подключении к sqlite", error)
    finally:
        if (sqlite_connection):
            sqlite_connection.close()
            print("Соединение с SQLite закрыто")

while(True):
    fromQueueToProgress()
    
    doneCheck()
    print ("I'm in infinity")
    time.sleep(5)
