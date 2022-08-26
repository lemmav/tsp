
import sqlite3, datetime

try:
    connection = sqlite3.connect('tspdb.db')
    queuequery = '''CREATE TABLE queue (
                                id INTEGER PRIMARY KEY,
                                url TEXT NOT NULL UNIQUE,
                                graph TEXT NOT NULL,
                                algorithm TEXT NOT NULL,
                                requesttime TEXT NOT NULL);'''

    cursor = connection.cursor()
    cursor.execute(queuequery)
    connection.commit()
    cursor.close()
 
    progressquery = '''CREATE TABLE progress (
                                id INTEGER PRIMARY KEY,
                                url TEXT NOT NULL UNIQUE,
                                graph TEXT NOT NULL,
                                algorithm TEXT NOT NULL,
                                starttime TEXT NOT NULL);'''

    cursor = connection.cursor()
    cursor.execute(progressquery)
    connection.commit()
    cursor.close()

    resultquery = '''CREATE TABLE result (
                                id INTEGER PRIMARY KEY,
                                url TEXT NOT NULL UNIQUE,
                                answer REAL NOT NULL,
                                algorithm TEXT NOT NULL,
                                executiontime TEXT NOT NULL);'''

    cursor = connection.cursor()
    cursor.execute(resultquery)
    connection.commit()
    cursor.close()

except sqlite3.Error as error:
    print("Ошибка при подключении к sqlite", error)
finally:
    if (connection):
        connection.close()
        print("Соединение с SQLite закрыто")