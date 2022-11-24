import string
from select import select
from flask import Flask, g, request, session, render_template, abort
from flask.helpers import url_for
import random, atexit
from datetime import timedelta, datetime, timezone
import requests, json
import re, sqlite3
from werkzeug.utils import redirect

server = Flask(__name__)

with open('./init.json', 'r', encoding='utf-8') as fh: #открываем файл на чтение
    algorithms = json.load(fh) #загружаем из файла данные в словарь dat

database_path = "/flask/createdbs.sqlite"

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(database_path)
        db.row_factory = sqlite3.Row
    return db

@server.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()

def query_db(query, args=(), one=False):
    cur = get_db().execute(query, args)
    rv = cur.fetchall()
    cur.close()
    return (rv[0] if rv else None) if one else rv

def execute_db_no_commit(query, args=()):
    cur = get_db().execute(query, args)
    cur.close()

def execute_db(query, args=()):
    execute_db_no_commit(query, args)
    get_db().commit()

def create_db():
    with server.app_context():
        db = get_db()
        cur = db.cursor()

        with server.open_resource('createdbs.sql', 'r') as f:
            script = f.read()
            cur.executescript(script)
        cur.close()
        db.commit()

def makeURL():
    domain="http://127.0.0.1:5000/"
    personalURL = ''.join(random.choices(string.ascii_letters + string.digits, k=10))
    return personalURL

@server.route("/graph",methods=['POST', 'GET'])
def main():
    if request.method == 'POST':
        print(request.form['graph'])
        while(True):
            url=makeURL()
            infograph = query_db(r"SELECT * FROM queue WHERE url=?", (url, ), True)
            if (infograph is None):
                execute_db(r"INSERT INTO queue (url, graph, algorithm, requesttime) VALUES (?, ?, ?, datetime('now', 'localtime'))",
                    (
                        url,
                        request.form['graph'],
                        request.form['algorithm'],
                        
                    ))
                return redirect(url_for('urlka',url=url))
        
    return render_template('graph.html')

@server.route('/graph/<url>',methods=['GET'])
def urlka(url):
    infographurlprogress = query_db(r"SELECT * FROM progress WHERE url=?", (url, ), True)
    infographurlresult = None
    if(infographurlprogress is not None):
        algorithmurl = infographurlprogress['algorithm']
        progressrequest = requests.get(algorithms[algorithmurl]+'/progress')
        progress = progressrequest.text
        print(progress)
        
    else:
        infographurlresult = query_db(r"SELECT * FROM result WHERE url=?", (url, ), True)
        progress=1
    return render_template('url.html', infographurlresult = infographurlresult, progress=progress )
    
create_db()
# if __name__ == "__main__":
#     create_db()
#     app.run()  






  