// Import von Bibliotheken
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definitionen des Displays
#define SCREEN_WIDTH 128 // OLED display Breite, in Pixeln
#define SCREEN_HEIGHT 64 // OLED display Höhe, in Pixeln

// Deklaration des Display-Objekts
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Definitionen der Pins
#define ROT_KFZ 2 // Pin für die rote KFZ-Ampel
#define GELB_KFZ 3 // Pin für die gelbe KFZ-Ampel
#define GRUEN_KFZ 4 // Pin für die grüne KFZ-Ampel
#define ROT_FG 5 // Pin für die rote Fußgängerampel
#define GRUEN_FG 6 // Pin für die grüne Fußgängerampel
#define TASTER 7 // Pin für den Taster
#define BUZZER 8 // Pin für den Buzzer

// Deklaration von Variablen
int zustand = 0; // Zustand der Ampel
bool buttonPressed = false; // Knopfstatus
int gruenphase = 5; // Countdown zur nächsten Grünphase für Fußgänger
int gruenphasezeit = 5; // Zeit der Fußgänger Grünphase

// Zeitvariablen für den Ablauf und das Buzzer-Intervall
unsigned long previousMillis = 0; 
unsigned long previousBuzzerMillis = 0; 
const long interval = 1000; // Intervallzeit in Millisekunden
const long buzzerInterval = 1000; // Buzzer-Intervallzeit in Millisekunden
unsigned long buttonDebounceTime = 0; // Debounce zeit für den Taster
const long debounceDelay = 50; // Verzögerung für den Debounce in Millisekunden
const long buzzerIntervalCase2 = 250; // 0.25 Sekunden Intervall für Case 2

// Initialisierung der Pins und des Displays
void setup() {
    pinMode(ROT_KFZ, OUTPUT);
    pinMode(GELB_KFZ, OUTPUT);
    pinMode(GRUEN_KFZ, OUTPUT);
    pinMode(ROT_FG, OUTPUT);
    pinMode(GRUEN_FG, OUTPUT);
    pinMode(TASTER, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    // Display initialisieren
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
}

// Anzeige des Ampelzustands auf dem Display und Steuerung der Ampel-LEDs
void zeigeAmpelZustand(const char* text, int rotKfz, int gelbKfz, int gruenKfz, int rotFg, int gruenFg) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(text);
    display.display();
    digitalWrite(ROT_KFZ, rotKfz);
    digitalWrite(GELB_KFZ, gelbKfz);
    digitalWrite(GRUEN_KFZ, gruenKfz);
    digitalWrite(ROT_FG, rotFg);
    digitalWrite(GRUEN_FG, gruenFg);
}

// Anzeige des Countdowns auf dem Display
void zeigeCountdown(const char* text, int counter) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(text);
    display.print(counter);
    display.display();
}

// Hauptschleife zur Steuerung der Ampel
void loop() {
    // Aktuelle Zeit in Millisekunden
    unsigned long currentMillis = millis();

    // Abfrage des Tasterzustands und Entprellung
    int buttonState = digitalRead(TASTER);
    if (buttonState == LOW && !buttonPressed && (currentMillis - buttonDebounceTime) > debounceDelay) {
        buttonPressed = true;
        buttonDebounceTime = currentMillis;
        zustand = 1; // Zustand ändern, wenn der Taster gedrückt wird
    } else if (buttonState == HIGH) {
        buttonPressed = false;
    }

    // Steuerung der Ampel basierend auf dem aktuellen Zustand
    switch (zustand) {
        case 0: // Normalzustand
            zeigeAmpelZustand("Rot", LOW, LOW, HIGH, HIGH, LOW);
            if (currentMillis - previousBuzzerMillis >= buzzerInterval) {
                previousBuzzerMillis = currentMillis;
                tone(BUZZER, 500, 500); // Buzzer alle 1000 ms aktivieren
            }
            break;

        case 1: // Knopf gedrückt
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                if (gruenphase >= 0) {
                    zeigeCountdown("Gruen in:", gruenphase);
                    tone(BUZZER, 500, 500); // Buzzer bei Countdown aktivieren
                    gruenphase--;
                } else {
                    zustand = 2; // Zustand ändern, wenn der Countdown abgelaufen ist
                    gruenphase = 5; // Countdown zurücksetzen
                }
            }
            break;

        case 2: // Fußgängerampel Grün
            zeigeAmpelZustand("Gruen", HIGH, LOW, LOW, LOW, HIGH);
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                if (gruenphasezeit > 0) {
                    zeigeCountdown("Gruen fuer:", gruenphasezeit);
                    gruenphasezeit--;
                } else {
                    noTone(BUZZER); // Buzzer ausschalten
                    zeigeAmpelZustand("", HIGH, HIGH, LOW, HIGH, LOW);
                    delay(1000);
                    zustand = 0; // Zustand zurücksetzen
                    gruenphasezeit = 5; // Countdown zurücksetzen
                    previousMillis = currentMillis;
                }
            }
            if (currentMillis - previousBuzzerMillis >= buzzerIntervalCase2) {
                previousBuzzerMillis = currentMillis;
                tone(BUZZER, 880, 250); // Buzzer alle 250 ms aktivieren
                delay(500);
            }
            break;
    }
}
