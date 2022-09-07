import sqlite3, datetime
import time, requests
algorithms = {'GA':'http://127.0.0.1:5001', 'BNN': 'http://127.0.0.1:5001', 'NN': 'http://127.0.0.1:5001'}
def read_sqlite_table():
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


while(True):
    read_sqlite_table()
    print ("I'm infinity")
    time.sleep(5)
