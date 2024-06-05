#include <Servo.h>

// Variable para controlar el servo
Servo servoRepEntrada;
Servo servoTalanquera;
Servo servoRepSalida;

// Definiendo pines digitales
const int s0 = 1;  
const int s1 = 2;  
const int s2 = 3;  
const int s3 = 4; 
const int out = 5;   

// Para almacenar niveles de cada color
int colorRojo = 0;  
int colorVerde = 0;  
int colorAzul = 0;  

// Contadores de paquetes
int cRojo = 0;
int cVerde = 0;
int cAmarillo = 0;
int cDefault = 0;
int cTotal = 0;

// Contador de esferas leídas
int eRojo = 0;
int eVerde = 0;
int eAmarillo = 0;
int eDefault = 0;
int eTotal = 0;

// Posiciones designadas para cada color
const int posRojo = 0;
const int posVerde = 160;
const int posAmarillo = 120;
const int posDefault = 60;

// Variable para almacenar la posición actual del servo
int currentPosition = posRojo;

// Flag para indicar si una esfera ha sido introducida
bool esferaIntroducida = false;

void setup() {    
  // Velocidad de transmisión para depuración 9600 bits por segundo
  Serial.begin(9600); 
  // Se definen los pines de salida
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  pinMode(s3, OUTPUT);  
  // Se define el pin out como entrada para leer los datos
  pinMode(out, INPUT);   
  // Se definen los pines en alto para aplicar el voltaje de alimentación
  digitalWrite(s0, HIGH);  
  digitalWrite(s1, HIGH); 
  // Inicializando servo en el pin 6, 7, 8
  servoRepEntrada.attach(6);
  servoTalanquera.attach(7);
  servoRepSalida.attach(8);
  servoRepSalida.writeMicroseconds(1500); // Detener el servo al inicio
  servoRepEntrada.write(0);
  servoTalanquera.write(0);
}  

void loop() { 
  // Verificar comandos desde el monitor serie
  comandosSeriales();

  if (esferaIntroducida) {
    // Validación para el contador de esferas que ya pasaron
    if(eTotal >= 10) {
      Serial.println("Proceso completado: 10 esferas procesadas.");
      Serial.println("Paquetes procesados: 3 pesados, 3 complejos, 3 livianos");
      Serial.println("Total de paquetes procesados: " + String(cTotal));
    }
    delay(500);  // Esperar un momento para que la esfera se estabilice
    color();  // Leer los colores después de la estabilización

    // Validando condiciones para comparar colores
    if (colorRojo < colorAzul && colorVerde > colorAzul && colorRojo < 48) { 
      if(eRojo > 2){
        Serial.println("Repositorio rojo lleno.");
      } else {
        cRojo += 16;
        eRojo++;
        eTotal++;
        Serial.println("Rojo - " + String(cRojo) + " bits - Pesado");
        delay(3000);
        // Mover el servoSalida de 0 a 90 grados y luego a 0
        activateServoTalanquera();
        moveToPosition(posRojo);
      }
    } else if (colorRojo > colorVerde && colorVerde <= colorAzul && colorVerde < 40) {   
      if(eVerde > 2){
        Serial.println("Repositorio verde lleno.");
      } else {
        cVerde += 12;
        eVerde++;
        eTotal++;
        Serial.println("Verde - " + String(cVerde) + " bits - Complejo"); 
        moveToPosition(posVerde);
        delay(3000);
        activateServoTalanquera();
        // Regresa a posición rojo
        moveToPosition(posRojo);
      }
    } else if (colorAzul > colorRojo && colorAzul > colorVerde && colorVerde > colorRojo) {   
      if(eAmarillo > 2){
        Serial.println("Repositorio amarillo lleno.");
      } else {
        cAmarillo += 8;
        eAmarillo++;
        eTotal++;
        Serial.println("Amarillo - " + String(cAmarillo) + " bits - Liviano");
        moveToPosition(posAmarillo);
        delay(3000);
        activateServoTalanquera();
        // Regresa a posición rojo
        moveToPosition(posRojo);
      }
    } else if (colorVerde > colorRojo && colorVerde > colorAzul && colorVerde < 76) {
      if(eDefault > 1){
        Serial.println("Repositorio default lleno.");
      } else {
        cDefault += 4;
        eDefault++;
        eTotal++;
        Serial.println("Default - " + String(cDefault) + " bits - Default");
        moveToPosition(posDefault);
        delay(3000);
        activateServoTalanquera();
        // Regresa a posición rojo
        moveToPosition(posRojo);
      }
    } else {
        Serial.println("Color no detectado.");
    }

    // Actualizar el contador total
    cTotal = cRojo + cVerde + cAmarillo + cDefault;

    // Resetear el flag de esfera introducida
    esferaIntroducida = false;
  }

  delay(3000); // Esperar 3 segundos antes de la siguiente lectura
} 

// Función para detectar colores
void color() {
  int readings = 5; // Número de lecturas para promediar
  int sumRojo = 0;
  int sumVerde = 0;
  int sumAzul = 0;

  for (int i = 0; i < readings; i++) {
    digitalWrite(s2, LOW);  
    digitalWrite(s3, LOW);   
    sumRojo += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);  
    digitalWrite(s3, HIGH);   
    sumAzul += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);  
    digitalWrite(s2, HIGH);    
    sumVerde += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);  
    delay(10); // Pequeño retardo entre lecturas
  }

  // Filtrar valores atípicos
  int threshold = 10000; // Umbral para considerar una lectura como ruido
  if (sumRojo / readings > threshold || sumVerde / readings > threshold || sumAzul / readings > threshold) {
    Serial.println("Lectura atípica detectada, repitiendo la lectura...");
    return; // En lugar de recursión, simplemente retorna y espera una nueva lectura
  }

  colorRojo = sumRojo / readings;
  colorVerde = sumVerde / readings;
  colorAzul = sumAzul / readings;
}

// Función para mover el repositorio de salida
void moveToPosition(int position) {
  if (position != currentPosition) {
    int angle = position - currentPosition;
    int pulseWidth;
    if (angle > 0) {
      pulseWidth = 1700; // Ajustar la velocidad correcta en dirección positiva
    } else {
      pulseWidth = 1300; // Ajustar la velocidad correcta en dirección negativa
      angle = -angle; // Convertir a positivo para el cálculo de delay
    }
    servoRepSalida.writeMicroseconds(pulseWidth);
    delay(map(angle, 0, 360, 0, 2000)); // Ajustar el tiempo de espera según el ángulo
    servoRepSalida.writeMicroseconds(1500); // Detener el servo después del movimiento
    currentPosition = position; // Actualizar la posición actual del servo
  }
}

// Movimiento de servo para repositorio de entrada
void activateServoEntrada() {
  servoRepEntrada.write(90);  
  delay(3000);           
  servoRepEntrada.write(0);   
  delay(2000);            
  esferaIntroducida = true;   
}

// Movimiento de servo para talanquera que libera esfera
void activateServoTalanquera() {
  servoTalanquera.write(90);  
  delay(2000);           
  servoTalanquera.write(0);  
  delay(2000);            
}

void comandosSeriales() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Eliminar espacios en blanco alrededor
    if (command == "push") {
      activateServoEntrada();
      Serial.println("Servo de entrada activado.");
    } else if (command == "reset") {
      resetCounters();
      // Serial.println("Contadores reiniciados y proceso reiniciado.");
      Serial.println("x - x - x");
    } else if (command == "release") {
      activateServoTalanquera();
      Serial.println("Habilitar talanquera.");
    }
  }
}

// Reinicia todo
void resetCounters() {
  // Reiniciar todos los contadores y variables
  cRojo = 0;
  cVerde = 0;
  cAmarillo = 0;
  cDefault = 0;
  eRojo = 0;
  eVerde = 0;
  eAmarillo = 0;
  eTotal = 0;
  cTotal = 0;  // Resetear cTotal también
  currentPosition = posRojo;
  moveToPosition(posRojo); // servo en la posición inicial
  esferaIntroducida = false; // Resetear el flag de esfera introducida
}