---
title: "IoT - Projet Serre connectée"
subtitle: "Rapport et documentation"
author: 
- Anthony Coke
- Guilain Mbayo
- Mehdi Salhi
date : \today
titlepage: true
logo: figures/logo.png
toc: true
toc-own-page: true
...

# Projet

Ce projet permet de réaliser une serre connectée. Il utilise des composants
Arduino pour prendre les mesures et Raspberry pour ingérer et afficher les
donnée.

# Matériel

Nous utilisons le matériel suivant: 

- Arduino MKR Wifi 1010 : exécution du code et connectique wifi
- Arduino MKR IOT Carrier : senseurs et actuateurs
- Raspberry Pi 4b : broker MQTT, base de donnée InfluxDB, interface web de
  gestion et configuration des appareils

## Senseurs

Nous utilisons les senseurs suivants qui permettent de prendre des mesures :

- Arduino MKR IOT Carrier
    - température : HTS221
    - humidité : HTS221
    - lumière : APDS-9960
- humidité du sol : Capacitive Soil Moisture Sensor v1.2
    - connecté à l'Arduino MRK IOT Carrier


## Actuateurs

Nous utilisons un actuateur : 

- ventilateur : Xilence XPF40.W DC12v 0.05A

# Logiciel

Les logiciels suivants sont utilisés :

- InfluxDB 2.71 : base de données
- Telegraf 1.26.3 : ingestion des données au format `line protocol` depuis le 
    broker MQTT vers la base de
  donnée InfluxDB
- Mosquitto 2.0.11-1 : broker MQTT
- Arduino : code arduino pour récupérer les mesures et communiquer avec le
  broker MQTT
- NodeJS 16.17.1 : serveur web d'administration
    - dépendances :
        - influxdata/influxdb-client": "^1.33.2"
        - tailwindcss/forms": "^0.5.3"
        - body-parser": "^1.20.2"
        - express": "^4.18.2"
        - express-requests-logger": "^4.0.0"
        - mqtt": "^4.3.7"
        - plotly.js": "^2.24.2"
        - tailwindcss": "^3.3.2"

# Architecture

![Architecture](figures/IOT_arch.drawio.png)


# Problèmes rencontrés

