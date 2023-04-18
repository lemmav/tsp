import string, yaml
import os
import sys
from select import select
from flask import Flask, g, request, session, render_template, abort
from flask.helpers import url_for
import random, atexit
from datetime import timedelta, datetime, timezone
import requests, json
import re, sqlite3
from werkzeug.utils import redirect


with open('./init.json', 'r', encoding='utf-8') as fh: #открываем файл на чтение
    algorithms = json.load(fh) #загружаем из файла данные в словарь dat

with open('./initcontainers.json', 'r', encoding='utf-8') as f: #открываем файл на чтение
    containers = json.load(f) #загружаем из файла данные в словарь dat

urltoip = {}
waitingpods={}
yamltodict={}
podcounter = 0
print (algorithms)
def fromYamltoDict(algorithm,counter):

    with open("ymls/"+algorithm+".yml", "r") as file:
        data = yaml.load(file, Loader=yaml.FullLoader)
        yamltodict[algorithm] = data
        yamltodict['BnB']['metadata']['name'] = algorithm+"-"+str(counter)
        yamltodict['BnB']['metadata']['labels']['app'] = algorithm+"-"+str(counter)
        yamltodict['BnB']['spec']['containers'][0]['name']=containers[algorithm]
        yamltodict['BnB']['spec']['containers'][0]['image']="docker.io/lemmav/"+containers[algorithm]
        yamltodict['BnB']['spec']['containers'][0]['ports'][0]['containerPort']=algorithms[algorithm][1:]+str(counter)
fromYamltoDict("BnB",1)

print (yamltodict)
