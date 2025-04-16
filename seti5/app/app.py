import os

from flask import Flask
from flask import request
from flask import send_file

import mysql.connector




app = Flask(__name__)

db_config = {
    'host': 'db',  # имя контейнера с MySQL
    'user': 'riba',
    'password': 'zaglotus',
    'database': 'doxed',
    'port': 3306
}

conn = mysql.connector.connect(**db_config)
cursor = conn.cursor()

def db_add(url):
    sql = "INSERT INTO urls (column1) VALUES (%s);"
    val = (url,)
    cursor.execute(sql, val)
    conn.commit()
    return 1



@app.route('/')
def hello():
    try:
        cursor.execute("SELECT VERSION()")
        version = cursor.fetchone()[0]
        return f"Connected to MySQL {version}"
    except Exception as e:
        return f"Error: {str(e)}", 500

@app.route('/add')
def push():
    target_url = request.args.get('url')
    db_add(target_url)
    return "added " + target_url +" sexfully"

@app.route('/get')
def pull():
    cursor.execute('SELECT * FROM urls;')
    rv = cursor.fetchall()
    json_file = open('database.json','w')
    json_file.write('{\n')
    for row in rv: 
        json_file.write('\t'+row[0]+',\n')
    json_file.write('}')
    json_file.close()
    return send_file('database.json',as_attachment=True,download_name='trojan.json')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)