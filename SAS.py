#code for Smart Attendance System which 
#uses api data to recieve data from the server
#and then uses the data to mark attendance
#of the students in the class

import requests
import json
import time
import datetime
import os
import sys
import time

#function to get the current time
def get_time():
    return time.strftime("%H:%M:%S")

#function to get the current date
def get_date():
    return time.strftime("%d/%m/%Y")

#function to get the current day
def get_day():
    return time.strftime("%A")


#import time table csv file
def import_timetable():
    with open('timetable.csv', 'r') as f:
        data = f.readlines()
    return data


# ThingSpeak credentials
channel_id = 2087297
write_api_key = 'S1TPYDW6T8Z0D2DE'
def delete_data():
    # URL for deleting all data from the channel
    url = 'https://api.thingspeak.com/channels/' + \
        str(channel_id) + '/feeds.json?api_key=' + write_api_key

    # Send a DELETE request to the URL
    response = requests.delete(url)

    # Check the response status code
    if response.status_code == 200:
        print('ThingSpeak data cleared successfully')
    else:
        print('Failed to clear ThingSpeak data')

def valid_duration():
    #if time matches with the time in the timetable
    time_list = []
    timetable = import_timetable()
    for i in range(1, len(timetable)):
        time_list.append(timetable[i].split(',')[0])
    #compare the time with the time in the timetable
    if (is_valid_time(time_list)):
        delete_data()
        return True
    else:
        return False
    
def is_valid_time(time_list):
    if get_time() in time_list:
        return True

def read_mapped_data():
    #open in read mode
    with open('mapped_data.csv', 'r') as f:
        data = f.readlines()
    return data

def get_data():
    url = 'https://api.thingspeak.com/channels/2087297/feeds.json?api_key=X77HJMZ0DL7XCV9A'
    response = requests.get(url)
    # Process the response and use the data as needed
    data = response.json()
    #selecting all the field1 data
    data = data['feeds']
    # Return the data or perform other actions with it
    return data

# search for the student id in the mapped data
def search_student_id(rfuid):
    data = read_mapped_data()
    data_split = data[0].split(',')
    # print(data_split)
    #iterate over the data by 2 steps
    if int(data_split[0]) == rfuid:
        # return the email of the student
        return data_split[2]
    else:
        print("No data found")
    for i in range(3, len(data_split), 2):
        print("UID: "+data_split[i])
        if int(data_split[i]) == rfuid:
            #return the email of the student
            return data_split[i+2]
    return None

def mark_attendance():
    while(True):
        data = get_data()
        for i in range(0, len(data)):
            #print the rfuid and the time of the student
            #format data[i]['created_at'] = 2020-04-27T12:00:00Z to only time
            data[i]['created_at'] = data[i]['created_at'].split('T')[1].split('Z')[0]
            print(data[i]['field1'], data[i]['created_at'])
            #search for the student id in the mapped data
            student_id = search_student_id(int(data[i]['field1']))
            # print(student_id)
            #if found then mark the attendance
            if student_id != None:
                #mark the attendance
                send_email(student_id, data[i]['created_at'])
                # print("emailed "+student_id)
        break

#read mapped data from the csv file and email the confirmation to student id

def send_email(student_email, student_time):
    # import the required libraries
    import smtplib
    from email.mime.multipart import MIMEMultipart
    from email.mime.text import MIMEText
    # send email to student email
    fromaddr = "attendancemonitorformitwpu@hotmail.com"
    toaddr = student_email
    # create message object instance
    msg = MIMEMultipart()
    # setup the parameters of the message
    msg['From'] = fromaddr
    msg['To'] = toaddr
    msg['Subject'] = "Attendance Confirmation"
    # add in the message body
    body = "Your attendance has been marked for the class on "+get_date()+" at "+get_time()+"."
    body = body + "Time of Tap : "+student_time+"\n"
    msg.attach(MIMEText(body, 'plain'))
    # create a SMTP session
    server = smtplib.SMTP('smtp.office365.com', 587)
    # start TLS for security
    server.starttls()
    # login to the email account
    server.login(fromaddr, "rr8664108@gmail.com")
    # convert message to string format
    text = msg.as_string()
    # send the message
    server.sendmail(fromaddr, toaddr, text)
    # close the SMTP session
    server.quit()

mark_attendance()
# delete_data()
