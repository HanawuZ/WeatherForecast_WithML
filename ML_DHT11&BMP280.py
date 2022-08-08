""" 
========================================================================
Process: 6/8/2022 XX:XX
Objectives:
    o/ ทำนายผลการพยากรณ์อากาศจากไฟล์ csv ได้โดยใช้ Scikit learn
    o/ ผลลัพธ์การทำนาย ได้แก่ "Less Cloudy", "Cloudy", "Sunny", "Humid", "Rainy" 
    o/ แสดงผลลัพธ์ประสิทธิ์ภาพโมเดล (Accuracy, Precision, Recall, F1-Score)
========================================================================
Process: 7/8/2022 XX:XX
Objectives:
    o? เก็บข้อมูลสภาพอากาศเพิ่มเติม เพื่อเพิ่มประสิทธิภาพของโมเดลให้สามารถทำนายได้แม่นยำขึ้น
    o? เพิ่มคำอธิบายโค๊ดให้ดูอ่านเข้าใจง่าย
========================================================================
"""

############################ Library Includes ############################
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.naive_bayes import GaussianNB
from sklearn import metrics

import matplotlib.pyplot as plt
##########################################################################

# +++++++++++++++++++++++++ Dataset Initialization +++++++++++++++++++++++++
""" ---------------------------------------------------------------
o These statements will initialize dataset from reading csv file
  and then initialize training dataset & test dataset
-------------------------------------------------------------------   """ 

# Read csv file using method read_csv() of pandas library
dataset = pd.read_csv("[Your CSV file location]");

# Initialize weather data values (Humidity, Temperature(C), Pressure(Pa), Altitude)
WeatherValue = dataset[["DHT-11 Humidity" ,"BMP-280 Temp" , 
                        "BMP-280 Pressure" , "BMP-280 Altitude"]].values.reshape(-4,4);

# Initialize weather condition ("Less Cloudy", "Cloudy", "Sunny", "Humid", "Rainy")
WeatherCondition = dataset["Weather Condition"].values.reshape(-1,1);

# Spliting dataset into training dataset & test dataset
WeatherValue_TrainingSet , WeatherValue_TestSet , WeatherCondition_TrainingSet , WeatherCondition_TestSet = train_test_split(WeatherValue,WeatherCondition , random_state = 0);


# +++++++++++++++++++++++++ Create Naive Bayes Model +++++++++++++++++++++++++
WeatherForecast_model = GaussianNB();               # Initialize Naive bayes model object                          

""" ---------------------------------------------------------------
o Train model by inserting Weather value training set & 
  Weather Condtion training set into Naive bayes model 
-------------------------------------------------------------------   """
WeatherForecast_model.fit(WeatherValue_TrainingSet , np.ravel(WeatherCondition_TrainingSet));   

# +++++++++++++++++++++++++ Prediction +++++++++++++++++++++++++
# Uses Naive Bayes Model Predict weather condition using Weather value test set
WeatherCondition_Predict = WeatherForecast_model.predict(WeatherValue_TestSet);    

# +++++++++++++++++++++++++ Test Accuracy +++++++++++++++++++++++++
# accuracy = metrics.accuracy_score(WeatherCondition_TestSet , WeatherCondition_Predict)*100;
# precision = metrics.precision_score(WeatherCondition_TestSet , WeatherCondition_Predict , average = "macro")*100;
# f1_score = metrics.f1_score(WeatherCondition_TestSet , WeatherCondition_Predict, average = "macro")*100;
# print("Accuracy Score = %f %%" % accuracy);
# print("Precision Score = %f %%" % precision);
# print("F1-Score = %f %%" % f1_score);

# Report Weather prediction using method 'classification_report'
print(metrics.classification_report(WeatherCondition_TestSet , WeatherCondition_Predict));