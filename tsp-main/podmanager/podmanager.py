import string, yaml, time
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
from kubernetes import client,config,utils

with open('./init.json', 'r', encoding='utf-8') as fh: #открываем файл на чтение
    algorithms = json.load(fh) #загружаем из файла данные в словарь dat
with open('./initcontainers.json', 'r', encoding='utf-8') as f: #открываем файл на чтение
    containers = json.load(f) #загружаем из файла данные в словарь dat

server = Flask(__name__)

urltoip = {} # хранит {url: (ip, name)}
waitingpods={} # хранит {algorithm: (ip, name)}
# yamltodict={}
total_pod_counter = 0 # !
active_pod_counter = 0 # !



def fromYamltoDict(algorithm,counter):
    with open("ymls/"+algorithm+".yml", "r") as file:
        data = yaml.load(file, Loader=yaml.FullLoader)

        data['metadata']['name'] = containers[algorithm]+"-"+str(counter)
        data['metadata']['labels']['app'] = containers[algorithm]+"-"+str(counter)
        data['spec']['containers'][0]['name']=containers[algorithm]
        data['spec']['containers'][0]['image']="docker.io/lemmav/"+containers[algorithm]
        data['spec']['containers'][0]['ports'][0]['containerPort']=int(algorithms[algorithm][1:])
    return data
        

def createPod(algorithm): # возвращает пару (pod_ip, pod_name)
    global total_pod_counter, active_pod_counter # !
    print("entered createPod")
    # config.load_kube_config()
    config.load_incluster_config()
    k8s_client = client.ApiClient()
    print("loaded API")
    # yaml_file = 'ymls/'+algorithm+".yml"
    print("podcounter = ", total_pod_counter)
    print("algorithm = ", algorithm)
    print("converting yaml")
    yamltodict =  fromYamltoDict(algorithm,total_pod_counter) # !
    print("creating pod")
    objects = utils.create_from_dict(k8s_client, yamltodict)
    print("created pod")
    # objects = utils.create_from_yaml(k8s_client,yaml_file,verbose=True)
    total_pod_counter += 1 # !
    active_pod_counter += 1 # !
    print("new podcounter = ", total_pod_counter)
    
    v1 = client.CoreV1Api()
    pod_name = yamltodict['metadata']['name'] # !
    while True:
        resp = v1.read_namespaced_pod(name=pod_name, namespace='default')
        if resp.status.phase != 'Pending':
            print("resp's pod_ip = ", resp.status.pod_ip)
            return (resp.status.pod_ip, pod_name) # !
        time.sleep(1)

def fillWaitingPods():
    print("entered fillWaitingPods")
    for algorithm in algorithms.keys():
        print("creating pod for ", algorithm)
        waitingpods[algorithm] = createPod(algorithm)
    print("finished fillWaitingPods")

fillWaitingPods()

# checkdb делает запрос на сервер podmanager, т.е. post-запрос
# podmanager в свою очерередь должен позволять по get-запросу собирать данные с него
@server.route("/",methods=['POST'])
def main():
    print("got a new request")
    if active_pod_counter<=5: # !
        url = request.args.get('url')
        algorithm = request.args.get('algorithm')
        graph = request.data
        pod = waitingpods[algorithm] # !
        pod_ip = pod[0] # !
        pod_name = pod[1] # можно заменить на pod_ip, pod_name = waitingpods[algorithm]
        requests.post("http://"+pod_ip+algorithms[algorithm]+"/graph", data=graph)
        print("sent to pod")

        urltoip[url] = (pod_ip, pod_name) # !
        
        
        print("creating new pod for waiting")
        waitingpods[algorithm] = createPod(algorithm) # TODO: добавить создание пода заранее
        # Т.е. необходимо чтобы был список ожидающих подов, запрос отправлялся на старый под, а на его место создавался новый
        
        print("handled request")
        return '1'
    else:
        print("too many pods")
        return '0'

@server.route("/progress",methods=['GET'])
def progress():
    url = request.args.get('url')
    algorithm = request.args.get('algorithm')
    pod_ip = urltoip[url][0] # !
    return requests.get("http://"+pod_ip+algorithms[algorithm]+'/progress').text

def deletePod(name):
    global active_pod_counter
    config.load_incluster_config()
    v1 = client.CoreV1Api()
    v1.delete_namespaced_pod(
            name=name, namespace="default",
            body={})
    active_pod_counter -= 1 # !

@server.route("/result",methods=['GET'])
def result():
    url = request.args.get('url')
    algorithm = request.args.get('algorithm')
    pod_ip = urltoip[url][0] # !
    pod_name = urltoip[url][1] # !
    result = requests.get("http://"+pod_ip+algorithms[algorithm]+'/result').text # !
    deletePod(pod_name) # !
    return result # !