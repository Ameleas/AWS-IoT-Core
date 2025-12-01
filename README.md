This repository is part of two in the "IoT and cloud services" course at Nackademin YH. Here I have taken sensor data: Humidity and light from an DHT22 sensor, plus LDR photo resistor to measure light and transferred the data to AWS IoTCore using mTLS.
To get started with AWS you first need to create an account: "https://signin.aws.amazon.com/signup?request_type=register". Notice that it is free to explore and test as a hobbyist but if you are not careful it can be an expensive experience. 
Therefore it is recommended to set up a "zero waste budget" so you at least get a notification when it starts costing you. Do so by following this link: "https://docs.aws.amazon.com/costmanagement/latest/userguide/budgets-managing-costs.html" and set alam level to 1 USD.
To connect the ESP32 to AWS Iot Core we followed this link: "https://www.hackster.io/mafzal/esp32-aws-iot-core-e6c274". The article is from 2022, but still worked fine at time of writing.

Security
IoT Core uses MQTT over mTLS for safe communication. MFA (Multi factor Authntication) is used to log on to you AWS-account. By adjusting the Policy attached to the device you can narrow what traffic to listen/send data to.

Scalability
You can easily add and delete devices to fit you needs. If you want to go beyond hobbyist level AWS supports JITP (just-in-time provisioning) as well as JITR (just-in-time registartion).



