from flask import Flask
from flask import request
from flask import send_file
import mysql.connector
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC


#database connection
mydb = mysql.connector.connect(host="localhost",
                               user="root",
                               password="zaglotus",
                               database="doxed",
                               auth_plugin = 'caching_sha2_password')
mycursor = mydb.cursor()

available = ['https://z-54.ru//catalog//zajigalki//']

def db_add(url,content):
    sql = "INSERT INTO sites (url,contents) VALUES (%s,%s);"
    val = (url,content)
    mycursor.execute(sql, val)
    mydb.commit()
    return 1

def parse_product(info):
    rows = info.split('\n')
    name = rows[0].replace(" 1*50*20",'')
    price = rows[1].split(' ')[0]
    amount = rows[2].split(' ')[4]
    if len(rows[3].split(' ')) == 6:
        block = rows[3].split(' ')[5]
    else:
        block = '0'
    return (name,price,amount,block)

def site_parse(url):
    if not(url in available):
        return url + " parsing not available"
    if url == available[0]:
        driver = webdriver.Chrome()
        outcome = []
        for page in range(1,8):
            driver.get(f"https://z-54.ru//catalog//zajigalki//?PAGEN_1={page}")
            elements = driver.find_elements(By.CLASS_NAME,"info-wrap")
        for element in elements:
            outcome.append(parse_product(element.text))
        to_write = ''
        for come in outcome:
            to_write += come[0]+' '+come[1]+' '+come[2]+' '+come[3]+'\n'
        db_add(url,to_write)
        return 'parsed sexfully'
            
app = Flask(__name__)


@app.route("/parse")
def push():
    target_url = request.args.get('url')
    logfile = open('log.txt','w')
    logfile.write(target_url)
    logfile.close()
    answer = site_parse(target_url)
    return answer

@app.route('/get')
def pull():
    mycursor.execute('SELECT * FROM sites;')
    rv = mycursor.fetchall()
    json_file = open('database.json','w')
    json_file.write('{\n')
    for row in rv: 
        json_file.write('\t'+row[0]+': '+row[1]+',\n')
    json_file.write('}')
    json_file.close()
    return send_file('database.json',as_attachment=True,download_name='trojan.json')