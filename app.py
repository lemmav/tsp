import os, string
from flask import Flask, g, request, session, render_template, abort
from flask.helpers import url_for
import bcrypt, base64
import random, atexit
from datetime import timedelta, datetime, timezone
import requests
import re
app = Flask(__name__)



@app.route("/graph",methods=['POST', 'GET'])
def main():
    if request.method == 'POST':
        inputgraph = request.form['graph']
        r = requests.post('http://127.0.0.1:5001/graph', data=inputgraph)
        return render_template('graph.html')

    return render_template('graph.html')

if __name__ == "__main__":
    app.run()  






  