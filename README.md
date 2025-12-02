IoT Sensor Data to AWS IoT Core
--------------------------------------

This repository is one of two created for the “IoT and Cloud Services” course at Nackademin YH.
In this project, an ESP32 collects humidity and temperature data from a DHT22 sensor, as well as light levels using an LDR photoresistor, and securely transmits the data to AWS IoT Core using MQTT over mTLS.

Getting Started with AWS
--------------------------------------

Before working with AWS IoT Core, you will need an AWS account:
https://signin.aws.amazon.com/signup?request_type=register

AWS offers a free tier, which is generally safe for hobby use, but unexpected costs can occur if services are left running.
To avoid surprises, it is strongly recommended to set up a cost-control budget:

AWS Zero-Waste Budget Guide:
https://docs.aws.amazon.com/costmanagement/latest/userguide/budgets-managing-costs.html

Set the budget alert threshold to 1 USD so you receive a notification if something starts generating cost.

Connecting ESP32 to AWS IoT Core
--------------------------------------

To connect the ESP32 to AWS IoT Core, the following guide was used and verified to still work at the time of writing (despite being from 2022):

ESP32 + AWS IoT Core Tutorial:
https://www.hackster.io/mafzal/esp32-aws-iot-core-e6c274

This guide covers device provisioning, certificate setup, and MQTT connection using mTLS.

Discord Online/Offline Notifications
--------------------------------------

To add online/offline notifications sent to a Discord channel, a Lambda function was created.
The code for the Lambda function is included in this repository.

Important:
You must replace the placeholder Webhook URL in the Lambda code with your own Discord webhook URL, which can be found under your Discord channel’s Integrations settings.

After creating the Lambda function:

Create an AWS IoT Core Rule

Configure the rule to trigger your Lambda function when the device connects or disconnects.

Device Shadows
---------------------------------------

This project also uses an AWS IoT Device Shadow, which provides a persistent, cloud-stored JSON document representing the device’s desired and reported state.
In this implementation, the device shadow is used to update how frequently the ESP32 sends sensor data. But it can be used for anything else as well.

When the desired state in the shadow changes (for example, a new reporting interval), the ESP32 receives an update, applies the new setting, and reports back its reported state. This ensures the device and the cloud always stay synchronized—even if the device temporarily goes offline.


Saving data
---------------------------------------
In this project I have used DynamoDB for hot storage and S3 for cold storage data. It is easy to cónnect to dynamoDB, you just search for it in you account in AWS.
After creating a table in DynamoDB you need to create a Rule in Message Routing in IoT Core that points to it. You also have to create an I Am role. Similarly you can create a bucket for long time storage in S3. Advances with DynamoBD is it is quick and direct access, drawback it is only saved for limited time. If you want to save data over long time, say 10 years, S3 is a bether choise.

Security
---------------------------------------

MQTT over mTLS ensures secure communication between the ESP32 and AWS IoT Core.

Multi-Factor Authentication (MFA) is used for secure access to your AWS account.

AWS IoT Policies allow fine-grained control over which topics a device can publish or subscribe to, improving security.

Scalability
----------------------------------------

AWS IoT Core is highly scalable — new devices can be added or removed easily.

For production-level provisioning AWS also supports:

JITP (Just-In-Time Provisioning)

JITR (Just-In-Time Registration)

These automation features allow large numbers of devices to securely onboard with minimal manual steps.

Related Project: Frontend with AWS Amplify
------------------------------------------

The second part of this project is a frontend built with AWS Amplify, which displays the sensor data.

You can find that repository here:
https://github.com/Ameleas/amplify-vite-react-template.git



