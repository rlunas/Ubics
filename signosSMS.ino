#include <SoftwareSerial.h>//incluimos la libreria SoftwareSerial
#include <TinyGPS.h>//incluimos la libreria TinyGPS para el modulo GPSMV2
#include <Wire.h> //Libreria que permite comunicarse con dispositivos por bus I2C
#include <MAX30105.h> //Libreria que permite obtener las lecturas de lu reflejada para calcular la frecuencia cardiaca 
#include <spo2_algorithm.h>
#include <heartRate.h>
#include <Adafruit_MLX90614.h>//Libreria que permite utilizar el sensor optico de temperatura
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); 
TinyGPS gps;//Declaramos el objeto gps
SoftwareSerial serialmovil(6, 5);//Declaramos el pin 6 Rx y 5 Tx
MAX30105 particleSensor; 
  int x=0;
  float latitud, longitud, temp; 
  const byte RATE_SIZE = 4; //numero que se utiliza para obtener promedio de lectura
  byte rates[RATE_SIZE]; //Matriz donde se almacenan las frecuencias cardiacas
  byte rateSpot = 0;
  long lastBeat = 0; //Hora de deteccion del ultimo latido
  float beatsPerMinute;
  int beatAvg,val;
  const int boton=2;
  int envio=0;
void setup()
{
  pinMode(boton,INPUT);
  Wire.begin();
  Serial.begin(9600);//Iniciamos el puerto serie
  serialmovil.begin(9600);//Iniciamos el puerto serie del modulo GA6 B GSM/GPRS
  while(!Serial);
}
void loop()
{ 
  ubicacion();
}
void frecuencia(){

      if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Utilizar el puerto I2C, velocidad 400kHz 
      {
        Serial.println("MAX3010 No funciona el Sensor ");
        while (1);
      }
          Serial.println("Coloca tu dedo en el sensor.");
          particleSensor.setup(); //Configurar el sensor con parametros predeterminados
          particleSensor.setPulseAmplitudeRed(0x0A); //Encender el led rojo para identificar su funcionamiento
          particleSensor.setPulseAmplitudeGreen(0); //Apagar el led verde
      while( x<1500 ){ 
        x=x+1;
          long irValue = particleSensor.getIR();
      
        if (checkForBeat(irValue) == true)
        {
          //Cuando se detecta un latido
          long delta = millis() - lastBeat;
          lastBeat = millis();
      
          beatsPerMinute = 60 / (delta / 1000.0);
      
          if (beatsPerMinute < 255 && beatsPerMinute > 20)
          {
            rates[rateSpot++] = (byte)beatsPerMinute; //Almacenar esta lectura en un arreglo
            rateSpot %= RATE_SIZE; //Ajustar el valor de la variable
      
            //Tomar el promedio de las lecturas
            beatAvg = 0;
            for (byte x = 0 ; x < RATE_SIZE ; x++)
              beatAvg += rates[x];
            beatAvg /= RATE_SIZE;
          }
        }
      
        Serial.print("IR=");
        Serial.print(irValue);
        Serial.print(", BPM=");
        Serial.print(beatsPerMinute);
        Serial.print(", Avg BPM=");
        Serial.print(beatAvg);
        val=digitalRead(boton); // detectar si el usuario pulsa o no el botón de ayuda
        if(val==HIGH){
              envio=1;
              temperatura();
              mensaje();
              delay(500);
            }
            else {
              envio=0;
            }
        if (irValue < 50000) //si la lectura de ir es menor a 50000 significa que no se ha colocado su dedo o muñeca en el sensor
          Serial.print(" No hay lectura?");
          Serial.println();
      }
}
void temperatura(){
  temp=0;
   mlx.begin(); 
  delay(500);
  temp=mlx.readObjectTempC();
  delay(500);
}
void ubicacion(){
 if (Serial.available())
  {
      int c = Serial.read();
      if(gps.encode(c))  
    {
      gps.f_get_position(&latitud, &longitud);
      Serial.print("La/Lo: "); 
      Serial.print(latitud,5); 
      Serial.print(", "); 
      Serial.println(longitud,5);
      delay(500);
      temperatura();
      delay(500);
      frecuencia();
      x=0;
      delay(500);
      mensaje();
      delay(3000);
    }
  }else{
    Serial.println("El sensor GPS no funciona");
  }
}
void mensaje(){
    serialmovil.println("AT+CMGF=1"); // Configuring modo TEXTO
    serialmovil.println("AT+CMGS=\"+527641114713\"");//cambiar codigo de pais +52 colocar numero a 10 digitos del telefono al que llegaran los sms 
    if(temp>37.2 or beatAvg>20 and beatAvg<50 or beatAvg>120){
          serialmovil.print("Temperatura o Frecuencia Fuera de Rango\n"); //COntenido del texto
          serialmovil.println("Ubicacion: https://maps.google.com/?q=");
          serialmovil.print(latitud,5);
          serialmovil.print(", ");
          serialmovil.print(longitud,5);
          serialmovil.println("\nLatidos: ");
          serialmovil.print(beatAvg);
          serialmovil.println("\nTemperatura: ");
          serialmovil.print(temp);
          serialmovil.write((char)26);
          delay(2000);
          Serial.println("Signos Vitales Fuera de Rango");
          Serial.println("\nUbicacion: ");
          Serial.print(latitud,5);
          Serial.print(" ,");
          Serial.print(longitud,5);
          Serial.println("\nLatidos: ");
          Serial.print(beatAvg);
          Serial.println("\nTemperatura: ");
          Serial.print(temp);
          serialmovil.write((char)26);
          delay(2000);
    }else  if(envio==1){
          serialmovil.println("Temperatura y Frecuencia Cardiaca Normal\n"); //COntenido del texto
          serialmovil.println("Ubicacion: https://maps.google.com/?q=");
          serialmovil.print(latitud,5);
          serialmovil.print(", ");
          serialmovil.print(longitud,5);
          serialmovil.println("\nLatidos: ");
          serialmovil.print(beatAvg);
          serialmovil.println("\nTemperatura: ");
          serialmovil.print(temp);
          serialmovil.write((char)26);
          delay(2000);
          Serial.println("Signos Vitales Normales");
          Serial.println("\nUbicacion: ");
          Serial.print(latitud,5);
          Serial.print(" ,");
          Serial.print(longitud,5);
          Serial.println("\nLatidos: ");
          Serial.print(beatAvg);
          Serial.println("\nTemperatura: ");
          Serial.println(temp);
    }else{
          Serial.println("Ubicacion: ");
          Serial.print(latitud,5);
          Serial.print(" ,");
          Serial.print(longitud,5);
          Serial.println("\nLatidos: ");
          Serial.print(beatAvg);
          Serial.println("\nTemperatura: ");
          Serial.println(temp);
      }
}
