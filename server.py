from flask import Flask, Response, render_template
from flask import request
from datetime import datetime
import pyrebase
import random
import json
import time

app = Flask(__name__)

config = {
    "apiKey": "AIzaSyAynKJF5yV1_HeWM9CnNO5Cm7-N9Kgryt4",
    "authDomain": "noisy-police.firebaseapp.com",
    "databaseURL": "https://noisy-police-default-rtdb.firebaseio.com/",
    "projectId": "noisy-police",
    "storageBucket": "noisy-police.appspot.com",
    "messagingSenderId": "589978085937",
    "appId": "1:589978085937:web:ab618471f554620a7ec310",
    "measurementId": "G-YQ0MQY9YK1"
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()

random.seed()  # Initialize the random number generator


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/chart-data')
def chartData():
    def generate():
        d = db.child("noise-data").order_by_child("timestamp").limit_to_last(30).get().val()
        for _, value in d.items():
            json_data = json.dumps(
                {'timestamp': value['timestamp'], 'noiseLevel': value['noiseLevel']})
            yield f"data:{json_data}\n\n"
            time.sleep(1)
        
    return Response(generate(), mimetype='text/event-stream')

'''
@app.route("/testput")
def testput():
    data = {
        "timestamp":str(datetime.now()),
        "noiseLevel": 500,
        "exceededThreshold": "N"
    }
    db.child("noise-data").push(data)
    return "Posted test data"
'''

@app.route("/sendData")
def sendData():
    noiseLevel = int(request.args.get("noiseLevel"))
    exceededThreshold = request.args.get("exceededThreshold")
    data = {
        "timestamp":str(datetime.now()),
        "noiseLevel": noiseLevel,
        "exceededThreshold": exceededThreshold
    }
    db.child("noise-data").push(data)
    return "Sent data:\nnoiseLevel ="+str(noiseLevel)+"\nexceededThreshold ="+exceededThreshold

if __name__ == "__main__":
    print("Starting Flask app")
    stream = db.child("noise_data").stream(stream_handler)

