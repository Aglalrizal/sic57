from flask import Flask, request, jsonify
from pymongo import MongoClient
from datetime import datetime
import logging

app = Flask(__name__)

# Ganti dengan URL koneksi MongoDB Atlas Anda
client = MongoClient("mongodb+srv://teguhrahmat911:edx4JgGgXvVvKdTb@kualitasudara.czsbrts.mongodb.net/?retryWrites=true&w=majority&appName=kualitasudara")
db = client['kualitasudara1'] # ganti sesuai dengan nama database kalian
collection = db['kualitasudara2'] # ganti sesuai dengan nama collections kalian

# Konfigurasi logging
logging.basicConfig(level=logging.DEBUG)

@app.route('/add', methods=['POST'])
def add_sensor_data():
    data = request.get_json()
    app.logger.debug(f"Received data: {data}")
    
    if not data:
        return jsonify({"error": "No data received"}), 400
    
    try:
        # Tambahkan timestamp ke data
        data['timestamp'] = datetime.utcnow()

        # Coba masukkan data ke MongoDB
        result = collection.insert_one(data)
        app.logger.debug(f"Data inserted with id: {result.inserted_id}")
        return jsonify({"message": "Data added successfully"}), 201
    except Exception as e:
        app.logger.error(f"Error inserting data: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/data', methods=['GET'])
def get_sensor_data():
    try:
        data = list(collection.find({}, {"_id": 0}))
        return jsonify(data)
    except Exception as e:
        app.logger.error(f"Error retrieving data: {e}")
        return jsonify({"error": str(e)}), 500



if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
