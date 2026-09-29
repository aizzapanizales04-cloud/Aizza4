#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <FirebaseClient.h>

#include <LittleFS.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <DHT.h>

#include <time.h>


/* =====================================================
   DHT11
   ===================================================== */

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(
    DHTPIN,
    DHTTYPE
);


/* =====================================================
   FIREBASE
   ===================================================== */

#define API_KEY \
    "AIzaSyCmdlAD8VWAhfQH_sS7ACSviLWF3M8IGak"

#define DATABASE_URL \
    "https://aizza-3d822-default-rtdb.firebaseio.com/"


/*
   IMPORTANT:

   Put your Firebase Authentication
   email and password here.

   Do NOT post the password publicly.
*/

#define USER_EMAIL \
    "aizzapanizales04@gmail.com"

#define USER_PASSWORD \
    "aizza0451"


/* =====================================================
   FIREBASE OBJECTS
   ===================================================== */

UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD,
    3000
);

FirebaseApp app;

WiFiClientSecure ssl_client;

AsyncClientClass aClient(
    ssl_client
);

RealtimeDatabase Database;


/* =====================================================
   WEB SERVER
   ===================================================== */

AsyncWebServer server(
    80
);


/* =====================================================
   WIFI MANAGER
   ===================================================== */

const char *AP_SSID =
    "AIZZA-ACT4-WIFI";

bool wifiManagerMode =
    false;


/* =====================================================
   SENSOR INTERVAL
   ===================================================== */

unsigned long lastSensorRead =
    0;


/*
   ACT4 records every 10 seconds.

   Change this if needed.
*/

const unsigned long SENSOR_INTERVAL =
    10000;


/* =====================================================
   FUNCTION DECLARATIONS
   ===================================================== */

void processFirebase(
    AsyncResult &aResult
);

bool connectToSavedWiFi();

void startWiFiManager();

void startMainWebServer();

void setupFirebase();

void sendSensorData();

String readFile(
    const char *path
);

bool writeFile(
    const char *path,
    const String &data
);

void deleteWiFiFiles();

String getDateString();

String getTimeString();


/* =====================================================
   READ FILE
   ===================================================== */

String readFile(
    const char *path
)
{

    if (
        !LittleFS.exists(path)
    )
    {
        return "";
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        return "";
    }


    String data =
        file.readString();


    file.close();


    data.trim();


    return data;
}


/* =====================================================
   WRITE FILE
   ===================================================== */

bool writeFile(
    const char *path,
    const String &data
)
{

    File file =
        LittleFS.open(
            path,
            "w"
        );


    if (!file)
    {

        Serial.print(
            "Failed to open: "
        );

        Serial.println(
            path
        );

        return false;
    }


    file.print(
        data
    );


    file.close();


    return true;
}


/* =====================================================
   DELETE WIFI FILES
   ===================================================== */

void deleteWiFiFiles()
{

    LittleFS.remove(
        "/ssid.txt"
    );

    LittleFS.remove(
        "/pass.txt"
    );

    LittleFS.remove(
        "/ip.txt"
    );

    LittleFS.remove(
        "/gateway.txt"
    );


    Serial.println(
        "WiFi settings deleted."
    );
}


/* =====================================================
   CONNECT SAVED WIFI
   ===================================================== */

bool connectToSavedWiFi()
{

    String ssid =
        readFile(
            "/ssid.txt"
        );

    String pass =
        readFile(
            "/pass.txt"
        );

    String ip =
        readFile(
            "/ip.txt"
        );

    String gateway =
        readFile(
            "/gateway.txt"
        );


    if (
        ssid.length() == 0
    )
    {

        Serial.println(
            "No saved WiFi."
        );

        return false;
    }


    Serial.println();
    Serial.println(
        "================================="
    );
    Serial.println(
        "      SAVED WIFI FOUND"
    );
    Serial.println(
        "================================="
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


    WiFi.mode(
        WIFI_STA
    );


    delay(
        500
    );


    /*
       STATIC IP
    */

    if (
        ip.length() > 0 &&
        gateway.length() > 0
    )
    {

        IPAddress local_IP;

        IPAddress gateway_IP;


        if (
            local_IP.fromString(
                ip
            ) &&
            gateway_IP.fromString(
                gateway
            )
        )
        {

            IPAddress subnet(
                255,
                255,
                255,
                0
            );


            if (
                WiFi.config(
                    local_IP,
                    gateway_IP,
                    subnet
                )
            )
            {

                Serial.println(
                    "Static IP configured."
                );

            }
            else
            {

                Serial.println(
                    "Static IP failed."
                );

            }

        }

    }


    WiFi.begin(
        ssid.c_str(),
        pass.c_str()
    );


    Serial.print(
        "Connecting"
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {

        Serial.print(
            "."
        );

        delay(
            500
        );
    }


    Serial.println();


    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {

        Serial.println();
        Serial.println(
            "================================="
        );
        Serial.println(
            "       WIFI CONNECTED"
        );
        Serial.println(
            "================================="
        );


        Serial.print(
            "IP Address: "
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "Gateway: "
        );

        Serial.println(
            WiFi.gatewayIP()
        );


        Serial.println();


        return true;
    }


    Serial.println(
        "WiFi connection failed."
    );


    WiFi.disconnect(
        true
    );


    delay(
        1000
    );


    return false;
}


/* =====================================================
   START WIFI MANAGER
   ===================================================== */

void startWiFiManager()
{

    wifiManagerMode =
        true;


    Serial.println();
    Serial.println(
        "================================="
    );
    Serial.println(
        "       AIZZA ACT4 WIFI MANAGER"
    );
    Serial.println(
        "================================="
    );


    WiFi.mode(
        WIFI_AP
    );


    delay(
        500
    );


    bool apStarted =
        WiFi.softAP(
            AP_SSID
        );


    if (!apStarted)
    {

        Serial.println(
            "Failed to start AP!"
        );

        return;
    }


    delay(
        1000
    );


    Serial.print(
        "AP SSID: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "AP IP: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    /* =================================================
       WIFI MANAGER PAGE
    ================================================= */

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            if (
                LittleFS.exists(
                    "/wifi-manager.html"
                )
            )
            {

                request->send(
                    LittleFS,
                    "/wifi-manager.html",
                    "text/html"
                );

            }
            else
            {

                request->send(
                    404,
                    "text/plain",
                    "wifi-manager.html not found."
                );

            }

        }
    );


    /* =================================================
       SAVE WIFI
    ================================================= */

    server.on(
        "/",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {

            String ssid = "";
            String pass = "";
            String ip = "";
            String gateway = "";


            if (
                request->hasParam(
                    "ssid",
                    true
                )
            )
            {

                ssid =
                    request
                    ->getParam(
                        "ssid",
                        true
                    )
                    ->value();

            }


            if (
                request->hasParam(
                    "pass",
                    true
                )
            )
            {

                pass =
                    request
                    ->getParam(
                        "pass",
                        true
                    )
                    ->value();

            }


            if (
                request->hasParam(
                    "ip",
                    true
                )
            )
            {

                ip =
                    request
                    ->getParam(
                        "ip",
                        true
                    )
                    ->value();

            }


            if (
                request->hasParam(
                    "gateway",
                    true
                )
            )
            {

                gateway =
                    request
                    ->getParam(
                        "gateway",
                        true
                    )
                    ->value();

            }


            ssid.trim();
            pass.trim();
            ip.trim();
            gateway.trim();


            if (
                ssid.length() == 0 ||
                pass.length() == 0
            )
            {

                request->send(
                    400,
                    "text/plain",
                    "SSID and password are required."
                );

                return;
            }


            writeFile(
                "/ssid.txt",
                ssid
            );


            writeFile(
                "/pass.txt",
                pass
            );


            writeFile(
                "/ip.txt",
                ip
            );


            writeFile(
                "/gateway.txt",
                gateway
            );


            request->send(
                200,
                "text/html",

                "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='"
                "font-family:Arial;"
                "background:#f5f6f8;"
                "color:#171717;"
                "padding:40px 20px;"
                "text-align:center;'>"

                "<div style='"
                "max-width:600px;"
                "margin:auto;"
                "background:white;"
                "border:1px solid #e5e7eb;"
                "border-radius:14px;"
                "padding:35px;'>"

                "<p style='"
                "font-size:11px;"
                "letter-spacing:1.5px;"
                "font-weight:bold;"
                "color:#888;'>"
                "AIZZA ACT4"
                "</p>"

                "<h1 style='"
                "font-weight:500;'>"
                "WiFi Saved"
                "</h1>"

                "<p style='color:#888;'>"
                "The ESP32 will restart and "
                "connect to the saved WiFi."
                "</p>"

                "<p style='color:#888;'>"
                "Please wait..."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );


            delay(
                1500
            );


            ESP.restart();

        }
    );


    server.begin();


    Serial.println();
    Serial.println(
        "WiFi Manager ready."
    );

    Serial.print(
        "Open: http://"
    );

    Serial.println(
        WiFi.softAPIP()
    );

    Serial.println();
}


/* =====================================================
   MAIN WEB SERVER
   ===================================================== */

void startMainWebServer()
{

    wifiManagerMode =
        false;


    /*
       MAIN DASHBOARD
    */

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            if (
                LittleFS.exists(
                    "/index.html"
                )
            )
            {

                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );

            }
            else
            {

                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );

            }

        }
    );


    /*
       CHANGE WIFI
    */

    server.on(
        "/change-wifi",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            request->send(
                200,
                "text/html",

                "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='"
                "font-family:Arial;"
                "background:#f5f6f8;"
                "padding:40px 20px;"
                "text-align:center;'>"

                "<div style='"
                "max-width:600px;"
                "margin:auto;"
                "background:white;"
                "border:1px solid #e5e7eb;"
                "border-radius:14px;"
                "padding:35px;'>"

                "<p style='"
                "font-size:11px;"
                "letter-spacing:1.5px;"
                "font-weight:bold;"
                "color:#888;'>"
                "AIZZA ACT4"
                "</p>"

                "<h1 style='font-weight:500;'>"
                "Changing WiFi..."
                "</h1>"

                "<p style='color:#888;'>"
                "WiFi settings will be cleared."
                "</p>"

                "<p style='color:#888;'>"
                "The ESP32 will restart."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );


            delay(
                1000
            );


            deleteWiFiFiles();


            ESP.restart();

        }
    );


    /*
       STATIC FILES
    */

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );


    server.begin();


    Serial.println();
    Serial.println(
        "================================="
    );
    Serial.println(
        "      MAIN WEB SERVER READY"
    );
    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.println();
}


/* =====================================================
   FIREBASE CALLBACK
   ===================================================== */

void processFirebase(
    AsyncResult &aResult
)
{

    if (
        !aResult.isResult()
    )
    {
        return;
    }


    if (
        aResult.isEvent()
    )
    {

        Firebase.printf(
            "Firebase Event - task: %s, "
            "msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.eventLog()
                .message()
                .c_str(),

            aResult.eventLog()
                .code()
        );
    }


    if (
        aResult.isDebug()
    )
    {

        Firebase.printf(
            "Firebase Debug - task: %s, "
            "msg: %s\n",

            aResult.uid().c_str(),

            aResult.debug().c_str()
        );
    }


    if (
        aResult.isError()
    )
    {

        Firebase.printf(
            "Firebase Error - task: %s, "
            "msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.error()
                .message()
                .c_str(),

            aResult.error()
                .code()
        );
    }


    if (
        aResult.available()
    )
    {

        Firebase.printf(
            "Firebase Payload - task: %s, "
            "payload: %s\n",

            aResult.uid().c_str(),

            aResult.c_str()
        );
    }
}


/* =====================================================
   FIREBASE SETUP
   ===================================================== */

void setupFirebase()
{

    Serial.println();
    Serial.println(
        "================================="
    );
    Serial.println(
        "       FIREBASE SETUP"
    );
    Serial.println(
        "================================="
    );


    /*
       SSL

       For school/lab testing.
       For production, use certificate
       validation instead.
    */

    ssl_client.setInsecure();


    Serial.println(
        "Initializing Firebase..."
    );


    initializeApp(
        aClient,
        app,
        getAuth(
            user_auth
        ),
        processFirebase,
        "authTask"
    );


    app.getApp<RealtimeDatabase>(
        Database
    );


    Database.url(
        DATABASE_URL
    );


    Serial.println(
        "Firebase initialization started."
    );

    Serial.println();
}


/* =====================================================
   GET DATE
   ===================================================== */

String getDateString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "1970-01-01";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(
        buffer
    );
}


/* =====================================================
   GET TIME
   ===================================================== */

String getTimeString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "00:00:00";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );


    return String(
        buffer
    );
}


/* =====================================================
   SEND SENSOR DATA
   ===================================================== */

void sendSensorData()
{

    /*
       Firebase ready?
    */

    if (
        !app.ready()
    )
    {

        Serial.println(
            "Firebase not ready yet..."
        );

        return;
    }


    /*
       READ DHT11
    */

    float humidity =
        dht.readHumidity();


    float temperature =
        dht.readTemperature();


    /*
       VALIDATE
    */

    if (
        isnan(humidity) ||
        isnan(temperature)
    )
    {

        Serial.println();
        Serial.println(
            "ERROR: Failed to read DHT11"
        );

        return;
    }


    /*
       DATE / TIME
    */

    String date =
        getDateString();


    String time =
        getTimeString();


    /*
       BASE PATH

       /ESP32_Data/YYYY-MM-DD/HH:MM:SS
    */

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;


    String temperaturePath =
        basePath +
        "/temperature";


    String humidityPath =
        basePath +
        "/humidity";


    /*
       SERIAL
    */

    Serial.println();
    Serial.println(
        "================================="
    );

    Serial.println(
        "       DHT11 SENSOR READING"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Temperature: "
    );

    Serial.print(
        temperature,
        1
    );

    Serial.println(
        " C"
    );


    Serial.print(
        "Humidity: "
    );

    Serial.print(
        humidity,
        1
    );

    Serial.println(
        " %"
    );


    Serial.print(
        "Date: "
    );

    Serial.println(
        date
    );


    Serial.print(
        "Time: "
    );

    Serial.println(
        time
    );


    Serial.print(
        "Base path: "
    );

    Serial.println(
        basePath
    );


    /*
       FIREBASE WRITE
    */

    Database.set<float>(
        aClient,
        temperaturePath,
        temperature,
        processFirebase,
        "temperatureTask"
    );


    Database.set<float>(
        aClient,
        humidityPath,
        humidity,
        processFirebase,
        "humidityTask"
    );


    Serial.println(
        "Sensor data sent to Firebase."
    );

    Serial.println(
        "================================="
    );

    Serial.println();
}


/* =====================================================
   SETUP
   ===================================================== */

void setup()
{

    Serial.begin(
        115200
    );


    delay(
        1000
    );


    Serial.println();
    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        " AIZZA ACT4 DHT11 FIREBASE"
    );

    Serial.println(
        " ENVIRONMENT MONITOR"
    );

    Serial.println(
        "================================="
    );


    /* =================================================
       LITTLEFS
    ================================================= */

    Serial.println(
        "Starting LittleFS..."
    );


    if (
        !LittleFS.begin(
            true
        )
    )
    {

        Serial.println(
            "LittleFS mount failed!"
        );


        while (true)
        {

            delay(
                1000
            );

        }

    }


    Serial.println(
        "LittleFS ready."
    );


    /* =================================================
       DHT11
    ================================================= */

    dht.begin();


    Serial.println(
        "DHT11 initialized."
    );


    /* =================================================
       WIFI
    ================================================= */

    bool connected =
        connectToSavedWiFi();


    if (!connected)
    {

        startWiFiManager();

        return;
    }


    /* =================================================
       NTP
    ================================================= */

    Serial.println(
        "Starting NTP..."
    );


    /*
       Philippines UTC+8
    */

    configTime(
        8 * 3600,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );


    Serial.print(
        "Waiting for time"
    );


    struct tm timeinfo;


    int retry = 0;


    while (
        !getLocalTime(
            &timeinfo
        ) &&
        retry < 20
    )
    {

        Serial.print(
            "."
        );

        delay(
            500
        );

        retry++;
    }


    Serial.println();


    if (
        getLocalTime(
            &timeinfo
        )
    )
    {

        Serial.println(
            "Time synchronized."
        );


        Serial.print(
            "Date: "
        );

        Serial.println(
            getDateString()
        );


        Serial.print(
            "Time: "
        );

        Serial.println(
            getTimeString()
        );

    }
    else
    {

        Serial.println(
            "WARNING: Time synchronization failed."
        );

    }


    /* =================================================
       FIREBASE
    ================================================= */

    setupFirebase();


    /* =================================================
       WEB SERVER
    ================================================= */

    startMainWebServer();


    /* =================================================
       READY
    ================================================= */

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "          SYSTEM READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

    Serial.println();
}


/* =====================================================
   LOOP
   ===================================================== */

void loop()
{

    /*
       Firebase async tasks
    */

    if (
        !wifiManagerMode
    )
    {

        app.loop();
    }


    /*
       SENSOR EVERY 10 SECONDS
    */

    if (
        !wifiManagerMode &&
        millis() -
        lastSensorRead >=
        SENSOR_INTERVAL
    )
    {

        lastSensorRead =
            millis();


        sendSensorData();
    }


    delay(
        10
    );
}
