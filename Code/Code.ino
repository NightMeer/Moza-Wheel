#include <Wire.h>

// =========================================================================
// KONFIGURATION & DEBUG SCHALTER
// =========================================================================
#define SLAVE_ADDRESS 0x09

// --- DEBUG SCHALTER (true = an, false = aus) ---
#define DEBUG_BUTTONS     true   // Zeigt an, wenn Knöpfe/Paddles gedrückt werden
#define DEBUG_CONNECTION  true   // Zeigt an, ob das Lenkrad mit der Base verbunden ist
#define DEBUG_I2C_TRAFFIC true  // WARNUNG: Zeigt JEDEN I2C Befehl an (Spammt den Serial Monitor voll!)

constexpr uint8_t FC_PAYLOAD = 0x04; // Moza ES ID
constexpr uint8_t F9_PAYLOAD = 0x02;

enum SlaveState { NONE, FC_RECEIVED, F9_RECEIVED, DD_RECEIVED, DB_RECEIVED, DE_RECEIVED };
volatile SlaveState currentState = NONE;
volatile int8_t btnSeqIdx = -1;

constexpr uint8_t payloadSize = 5;
volatile uint8_t btnPayloads[payloadSize] = {0, 0, 0, 0, 0};
volatile uint8_t rotaryPayloads[payloadSize] = {0, 0, 0, 0, 0};
volatile uint8_t paddlePayloads[payloadSize] = {0, 0, 0, 0, 0};

// --- Debug-Speicher ---
uint8_t lastBtnPayloads[payloadSize] = {0};
uint8_t lastPaddlePayloads[payloadSize] = {0};

// --- Verbindungs- & Traffic-Speicher ---
volatile unsigned long lastI2cTime = 0;
bool isConnected = false;

// Ringpuffer für I2C Traffic (sichert Daten zwischen ISR und Loop)
constexpr uint8_t BUFFER_SIZE = 64;
volatile uint8_t i2cRxBuffer[BUFFER_SIZE];
volatile uint8_t i2cRxHead = 0;
volatile uint8_t i2cRxTail = 0;

// =========================================================================
// MAPPING STRUKTUREN
// =========================================================================

struct ButtonMapping {
    uint8_t pin;    
    uint8_t index;  
    uint8_t value;  
};

struct PaddleMapping {
    uint8_t pin;       
    uint8_t data[5];   
};

// --- BUTTON MAPPING ---
const ButtonMapping myButtons[] = {
    {0, 1, 0x40}, {0, 1, 0x80}, {0, 1, 0x20}, {6, 1, 0x10}, {0, 0, 0x80}, 
    {9, 0, 0x40}, {4, 0, 0x10}, {5, 0, 0x20}, {0, 2, 0x40}, {7, 2, 0x80}, 
    {8, 2, 0x10}, {0, 2, 0x20}, {0, 3, 0x10}, {0, 3, 0x08}, {14, 3, 0x40}, 
    {16, 3, 0x80}, {10, 3, 0x20}, {0, 4, 0x80}, {0, 4, 0x40}, {A1, 4, 0x10}, 
    {A0, 4, 0x20}, {15, 4, 0x08}
};

// --- PADDLE MAPPING ---
const PaddleMapping myPaddles[] = {
    {A3, {0xC2, 0xC2, 0xC0, 0xC0, 0xC2}}, 
    {A2, {0xC4, 0xC4, 0xC0, 0xC0, 0xC4}}
};

const size_t numButtons = sizeof(myButtons) / sizeof(ButtonMapping);
const size_t numPaddles = sizeof(myPaddles) / sizeof(PaddleMapping);

// =========================================================================
// I2C LOGIK (Interrupts)
// =========================================================================

void requestEvent() {
    lastI2cTime = millis(); // Update Verbindungs-Timer
    
    uint8_t response = 0;
    switch (currentState) {
        case FC_RECEIVED: response = FC_PAYLOAD; break;
        case F9_RECEIVED: response = F9_PAYLOAD; break;
        case DD_RECEIVED:
            if (btnSeqIdx >= 0 && btnSeqIdx < payloadSize) {
                response = btnPayloads[btnSeqIdx] + rotaryPayloads[btnSeqIdx];
            }
            break;
        case DE_RECEIVED:
            if (btnSeqIdx >= 0 && btnSeqIdx < payloadSize) {
                response = paddlePayloads[btnSeqIdx];
            }
            break;
        case DB_RECEIVED: response = 0x04; break;
        default: response = 0; break;
    }
    Wire.write(response);
    currentState = NONE;
}

void receiveEvent(int bytes) {
    lastI2cTime = millis(); // Update Verbindungs-Timer

    while (Wire.available()) {
        uint8_t received = Wire.read();

        // Wenn Traffic-Debug an ist, Befehl in den Puffer schreiben
        if (DEBUG_I2C_TRAFFIC) {
            uint8_t nextHead = (i2cRxHead + 1) % BUFFER_SIZE;
            if (nextHead != i2cRxTail) { // Nur schreiben, wenn Puffer nicht voll
                i2cRxBuffer[i2cRxHead] = received;
                i2cRxHead = nextHead;
            }
        }

        switch (received) {
            case 0xFC: currentState = FC_RECEIVED; btnSeqIdx = -1; break;
            case 0xF9: currentState = F9_RECEIVED; break;
            case 0xDD: currentState = DD_RECEIVED; btnSeqIdx++; break;
            case 0xDE: currentState = DE_RECEIVED; break;
            case 0xDB: currentState = DB_RECEIVED; break;
            default: currentState = NONE; break;
        }
    }
}

// =========================================================================
// DEBUG FUNKTIONEN FÜR DEN LOOP
// =========================================================================

void printHex(uint8_t num) {
    if (num < 0x10) Serial.print("0");
    Serial.print(num, HEX);
}

void processDebug() {
    // --- 1. VERBINDUNGS-STATUS PRÜFEN ---
    if (DEBUG_CONNECTION) {
        unsigned long currentI2cTime;
        // Sicherer Lesezugriff auf Interrupt-Variable
        noInterrupts(); 
        currentI2cTime = lastI2cTime; 
        interrupts();

        // Wenn das letzte Signal weniger als 500ms her ist, sind wir verbunden
        bool currentlyConnected = (millis() - currentI2cTime) < 500;
        
        // Nur bei Änderung ausgeben
        if (currentlyConnected != isConnected) {
            isConnected = currentlyConnected;
            if (isConnected) {
                Serial.println("\n[STATUS] >>> WHEEL BASE VERBUNDEN <<<");
            } else {
                Serial.println("\n[STATUS] >>> WHEEL BASE GETRENNT <<<");
            }
        }
    }

    // --- 2. I2C TRAFFIC AUSGEBEN ---
    if (DEBUG_I2C_TRAFFIC) {
        while (i2cRxTail != i2cRxHead) {
            uint8_t cmd = i2cRxBuffer[i2cRxTail];
            i2cRxTail = (i2cRxTail + 1) % BUFFER_SIZE;
            
            Serial.print("[I2C] Empfangen: 0x");
            printHex(cmd);
            Serial.print(" -> ");
            
            // Befehl übersetzen
            switch(cmd) {
                case 0xFC: Serial.println("ID Abfrage (FC)"); break;
                case 0xF9: Serial.println("Setup Abfrage (F9)"); break;
                case 0xDD: Serial.println("Lese Buttons (DD)"); break;
                case 0xDE: Serial.println("Lese Paddles (DE)"); break;
                case 0xDB: Serial.println("Zyklus Ende (DB)"); break;
                default:   Serial.println("Unbekannter Befehl"); break;
            }
        }
    }

    // --- 3. BUTTON & PADDLE STATUS ---
    if (DEBUG_BUTTONS) {
        bool hasChanged = false;
        for (int i = 0; i < payloadSize; i++) {
            if (btnPayloads[i] != lastBtnPayloads[i] || paddlePayloads[i] != lastPaddlePayloads[i]) {
                hasChanged = true;
                break;
            }
        }

        if (hasChanged) {
            Serial.print("[DATA] BTN: [");
            for (int i = 0; i < payloadSize; i++) {
                printHex(btnPayloads[i]);
                if (i < payloadSize - 1) Serial.print(" ");
                lastBtnPayloads[i] = btnPayloads[i]; 
            }
            Serial.print("] | PAD: [");
            for (int i = 0; i < payloadSize; i++) {
                printHex(paddlePayloads[i]);
                if (i < payloadSize - 1) Serial.print(" ");
                lastPaddlePayloads[i] = paddlePayloads[i]; 
            }
            Serial.println("]");
        }
    }
}

// =========================================================================
// SETUP & LOOP
// =========================================================================

void setup() {
    Serial.begin(115200);
    
    Wire.begin(SLAVE_ADDRESS);
    Wire.setClock(400000);
    digitalWrite(SDA, LOW);
    digitalWrite(SCL, LOW);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    for (size_t i = 0; i < numButtons; i++) {
        if (myButtons[i].pin > 0) pinMode(myButtons[i].pin, INPUT_PULLUP);
    }
    
    for (size_t i = 0; i < numPaddles; i++) {
        if (myPaddles[i].pin > 0) pinMode(myPaddles[i].pin, INPUT_PULLUP);
    }
    
    Serial.println("=========================================");
    Serial.println("  Moza ES Emulator gestartet");
    Serial.println("=========================================");
    Serial.println("Warte auf Verbindung zur Wheel Base...");
    
    if (DEBUG_I2C_TRAFFIC) {
        Serial.println("ACHTUNG: I2C Traffic Debugging ist AN. Das wird extrem viel Text erzeugen!");
    }
}

void loop() {
    uint8_t tempBtnPayloads[payloadSize] = {0};
    uint8_t tempPaddlePayloads[payloadSize] = {0};

    // 1. Buttons scannen
    for (size_t i = 0; i < numButtons; i++) {
        if (myButtons[i].pin > 0) {
            if (digitalRead(myButtons[i].pin) == LOW) {
                tempBtnPayloads[myButtons[i].index] |= myButtons[i].value;
            }
        }
    }

    // 2. Paddles scannen
    for (size_t i = 0; i < numPaddles; i++) {
        if (myPaddles[i].pin > 0) {
            if (digitalRead(myPaddles[i].pin) == LOW) {
                for (int j = 0; j < 5; j++) {
                    tempPaddlePayloads[j] += myPaddles[i].data[j];
                }
            }
        }
    }

    // 3. Payloads übergeben
    noInterrupts();
    for (int i = 0; i < payloadSize; i++) {
        btnPayloads[i] = tempBtnPayloads[i];
        paddlePayloads[i] = tempPaddlePayloads[i];
    }
    interrupts();

    // 4. Debug-Ausgaben verarbeiten
    processDebug();

    delay(5);
}