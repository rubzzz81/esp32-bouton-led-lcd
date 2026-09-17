#include <Arduino.h>
#include <Wire.h>
#include <rgb_lcd.h>

#define LED_PIN    2   // LED (souvent la LED intégrée sur les devkits ESP32-WROOM-32)
#define BUTTON_PIN 4   // Bouton (câblé entre la broche et GND)

rgb_lcd lcd;

struct Status {
  const char* line0;
  const char* line1;
  uint8_t r, g, b;
};

// Accents évités volontairement ("journee" sans é) : les caractères
// accentués UTF-8 ne s'affichent pas correctement sur ce type d'écran LCD.
const Status STATUSES[] = {
  { "Libre, frappe",  "et entre !",   0,   255, 0   }, // vert : disponible
  { "En",             "consultation", 255, 165, 0   }, // orange : occupe
  { "Absent reviens", "plus tard",    255, 60,  0   }, // orange fonce : absence courte
  { "Absent toute",   "la journee",   255, 0,   0   }, // rouge : absence longue
};
const uint8_t STATUS_COUNT = sizeof(STATUSES) / sizeof(STATUSES[0]);

uint8_t statusIndex = 0;
bool lastButtonState = false; // état brut précédent, pour détecter le front de l'appui

void showStatus(uint8_t index) {
  const Status &s = STATUSES[index];
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(s.line0);
  lcd.setCursor(0, 1);
  lcd.print(s.line1);
  lcd.setRGB(s.r, s.g, s.b);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Assure que la LED est bien éteinte au démarrage
  pinMode(BUTTON_PIN, INPUT_PULLUP); // pull-up interne, donc LOW = bouton appuyé

  Wire.begin(); // SDA = GPIO21, SCL = GPIO22 par défaut sur l'ESP32
  lcd.begin(16, 2);
  showStatus(statusIndex);
}

void loop() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Front montant : le statut avance d'un cran à chaque appui (pas au relâchement)
  if (buttonPressed && !lastButtonState) {
    statusIndex = (statusIndex + 1) % STATUS_COUNT;
    showStatus(statusIndex);
    digitalWrite(LED_PIN, statusIndex == 0 ? LOW : HIGH); // LED allumée = "non libre"
    delay(50); // anti-rebond basique
  }

  lastButtonState = buttonPressed;
}
