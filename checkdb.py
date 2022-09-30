import sqlite3, datetime
import time, requests, json
with open('init.json', 'r', encoding='utf-8') as fh: #открываем файл на чтение
    algorithms = json.load(fh) #загружаем из файла данные в словарь data
    
def fromqueuetoprogress():
    try:
        sqlite_connection= sqlite3.connect('createdbs.sqlite', timeout=20)
        sqlite_connection.row_factory = sqlite3.Row
        cursor = sqlite_connection.cursor()
        print("Подключен к SQLite")
        for algorithm in algorithms.keys():
            cursor.execute("SELECT * FROM progress WHERE algorithm=?", (algorithm,))
            progressrow = cursor.fetchone()
            print("SELECTED FROM progress")
            if progressrow is None:
                cursor.execute("SELECT * FROM queue WHERE algorithm=? ORDER BY requesttime", (algorithm,))
                print("SELECTED FROM queue")
                queuerow = cursor.fetchone()
                if queuerow is not None:
                    print(queuerow['graph'])
                    cursor.execute("INSERT INTO progress (url, graph, algorithm, starttime) VALUES (?, ?, ?, datetime('now', 'localtime')) ", (queuerow['url'], queuerow['graph'], queuerow['algorithm']))
                    cursor.execute("DELETE FROM queue WHERE url=?", (queuerow['url'],))
                    print("INSERTED INTO progress and DELETED FROM queue")
                    sqlite_connection.commit()
                    requests.post(algorithms[queuerow['algorithm']]+"/graph", data=queuerow['graph'])
        cursor.close()

    except sqlite3.Error as error:
        print("Ошибка при подключении к sqlite", error)
    finally:
        if (sqlite_connection):
            sqlite_connection.close()
            print("Соединение с SQLite закрыто")

def donecheck():
    try:
        sqlite_connection= sqlite3.connect('createdbs.sqlite', timeout=20)
        sqlite_connection.row_factory = sqlite3.Row
        cursor = sqlite_connection.cursor()
        print("Подключен к SQLite")
        for algorithm in algorithms.keys():
            cursor.execute("SELECT * FROM progress WHERE algorithm=?", (algorithm,))
            progressrow = cursor.fetchone()
            print("SELECTED FROM progress")
            if progressrow is not None:
                progressrequest = requests.get(algorithms[algorithm]+'/progress')
                if int(float(progressrequest.text))==1:
                    resultrequest = requests.get(algorithms[algorithm]+'/result')
                    resultrequest_str = resultrequest.text
                    resultrequest_str = resultrequest_str.split('\n')
                    print(resultrequest_str[-1])
                    cursor.execute("INSERT INTO result (url, answer, way, algorithm, executiontime) VALUES (?, ?, ?, ?, datetime('now', 'localtime')) ", (progressrow['url'], resultrequest_str[-1], resultrequest_str[1], progressrow['algorithm']))
                    cursor.execute("DELETE FROM progress WHERE url=?", (progressrow['url'],))
                    print("INSERTED INTO result and DELETED FROM progress")
                    sqlite_connection.commit()

        cursor.close()

    except sqlite3.Error as error:
        print("Ошибка при подключении к sqlite", error)
    finally:
        if (sqlite_connection):
            sqlite_connection.close()
            print("Соединение с SQLite закрыто")

while(True):
    fromqueuetoprogress()
    
    donecheck()
    print ("I'm in infinity")
    time.sleep(5)
