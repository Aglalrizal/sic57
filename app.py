from flask import Flask, jsonify, request
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from bson.json_util import dumps, ObjectId
from datetime import datetime, timedelta
import random

app = Flask(__name__)

uri = "mongodb+srv://teguhrahmat911:edx4JgGgXvVvKdTb@kualitasudara.czsbrts.mongodb.net/?retryWrites=true&w=majority&appName=kualitasudara"
# Create a new client and connect to the server
client = MongoClient(uri, server_api=ServerApi('1'))
try:
    client.admin.command('ping')
    print("Pinged your deployment. You successfully connected to MongoDB!")
except Exception as e:
    print(e)

db = client['kualitasudara1']  # ganti sesuai dengan nama database kalian
my_collection = db['kualitasudara2']  # ganti sesuai dengan nama collections kalian

@app.route('/latest_data', methods=['GET'])
def get_latest_data():
    data_terakhir = my_collection.find().sort('_id', -1).limit(1)

    # Mengambil dokumen pertama dari kursor
    latest_data = next(data_terakhir, None)

    if latest_data:
        # Mengonversi ObjectId menjadi string
        latest_data['_id'] = str(latest_data['_id'])
        return latest_data
    else:
        print("Tidak ada data ditemukan.")

@app.route('/all_data', methods=['GET'])
def get_data():
    start_date = request.args.get('start_date')
    end_date = request.args.get('end_date')
    
    query = {}
    
    if start_date:
        query['timestamp'] = {'$gte': datetime.fromisoformat(start_date)}
    if end_date:
        if 'timestamp' in query:
            query['timestamp']['$lte'] = datetime.fromisoformat(end_date)
        else:
            query['timestamp'] = {'$lte': datetime.fromisoformat(end_date)}
    
    data = list(my_collection.find(query))
    
    for item in data:
        item['_id'] = str(item['_id'])
    
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True)
