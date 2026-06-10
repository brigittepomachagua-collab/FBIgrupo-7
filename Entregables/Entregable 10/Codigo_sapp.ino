#include <Wire.h>
#include <MPU6050_tockn.h>
#include <math.h>

#define TCAADDR 0x70

MPU6050 mpu1(Wire);
MPU6050 mpu2(Wire);
MPU6050 mpu3(Wire);

// Variables de alerta
unsigned long inicioMalaPostura = 0;
unsigned long tiempoMalaPostura = 0;

bool contandoMalaPostura = false;
bool alertaActiva = false;

int numeroAlertas = 0;

// Función multiplexor
void tcaSelect(uint8_t i)
{
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

// Ángulos actuales
float imu1X, imu1Y;
float imu2X, imu2Y;
float imu3X, imu3Y;

// Ángulos de referencia
float ref1X, ref1Y;
float ref2X, ref2Y;
float ref3X, ref3Y;

// Variables de postura
bool calibrado = false;

float error1;
float error2;
float error3;
float errorMax;

String estado = "SIN CALIBRAR";

// Setup
void setup()
{
  Serial.begin(115200);

  Wire.begin(21, 22);

  // IMU 1
  tcaSelect(0);
  mpu1.begin();
  mpu1.calcGyroOffsets(true);

  // IMU 2
  tcaSelect(1);
  mpu2.begin();
  mpu2.calcGyroOffsets(true);

  // IMU 3
  tcaSelect(2);
  mpu3.begin();
  mpu3.calcGyroOffsets(true);

  Serial.println("=================================");
  Serial.println("SPINEGUARD INICIADO");
  Serial.println("Presiona C para calibrar");
  Serial.println("=================================");
}

// Calibración
void calibrarPostura()
{
  ref1X = imu1X;
  ref1Y = imu1Y;

  ref2X = imu2X;
  ref2Y = imu2Y;

  ref3X = imu3X;
  ref3Y = imu3Y;

  calibrado = true;

  numeroAlertas = 0;
  contandoMalaPostura = false;
  alertaActiva = false;
  tiempoMalaPostura = 0;

  Serial.println();
  Serial.println("===== POSTURA CALIBRADA =====");

  Serial.print("IMU1 -> X: ");
  Serial.print(ref1X);
  Serial.print(" Y: ");
  Serial.println(ref1Y);

  Serial.print("IMU2 -> X: ");
  Serial.print(ref2X);
  Serial.print(" Y: ");
  Serial.println(ref2Y);

  Serial.print("IMU3 -> X: ");
  Serial.print(ref3X);
  Serial.print(" Y: ");
  Serial.println(ref3Y);

  Serial.println("=============================");
}

// Evaluación de postura
void evaluarPostura()
{
  if (!calibrado)
  {
    estado = "SIN CALIBRAR";
    return;
  }

  error1 = max(
    fabs(imu1X - ref1X),
    fabs(imu1Y - ref1Y)
  );

  error2 = max(
    fabs(imu2X - ref2X),
    fabs(imu2Y - ref2Y)
  );

  error3 = max(
    fabs(imu3X - ref3X),
    fabs(imu3Y - ref3Y)
  );

  errorMax = max(
    error1,
    max(error2, error3)
  );

  if (errorMax <= 5)
  {
    estado = "NORMAL";
  }
  else if (errorMax <= 10)
  {
    estado = "LEVE";
  }
  else
  {
    estado = "MALA";
  }
}

// Verificación de alerta
void verificarAlerta()
{
  if (estado == "MALA")
  {
    if (!contandoMalaPostura)
    {
      inicioMalaPostura = millis();
      contandoMalaPostura = true;
    }

    tiempoMalaPostura =
      (millis() - inicioMalaPostura) / 1000;

    if ((millis() - inicioMalaPostura >= 15000)
        && !alertaActiva)
    {
      alertaActiva = true;

      numeroAlertas++;

      Serial.println();
      Serial.println("***** ALERTA DE POSTURA *****");
      Serial.print("Numero de alertas: ");
      Serial.println(numeroAlertas);
      Serial.println();
    }
  }
  else
  {
    contandoMalaPostura = false;
    alertaActiva = false;
    tiempoMalaPostura = 0;
  }
}

// Loop principal
void loop()
{
  //IMU 1 
  tcaSelect(0);
  mpu1.update();

  imu1X = mpu1.getAngleX();
  imu1Y = mpu1.getAngleY();

  // IMU 2 
  tcaSelect(1);
  mpu2.update();

  imu2X = mpu2.getAngleX();
  imu2Y = mpu2.getAngleY();

  // IMU 3 
  tcaSelect(2);
  mpu3.update();

  imu3X = mpu3.getAngleX();
  imu3Y = mpu3.getAngleY();

  // Procesamiento 
  evaluarPostura();
  verificarAlerta();

  //Monitor Serie

  Serial.print("IMU1 X:");
  Serial.print(imu1X);
  Serial.print(" Y:");
  Serial.print(imu1Y);

  Serial.print(" | IMU2 X:");
  Serial.print(imu2X);
  Serial.print(" Y:");
  Serial.print(imu2Y);

  Serial.print(" | IMU3 X:");
  Serial.print(imu3X);
  Serial.print(" Y:");
  Serial.println(imu3Y);

  Serial.print("Estado: ");
  Serial.println(estado);

  if (calibrado)
  {
    Serial.print("Error IMU1: ");
    Serial.print(error1);

    Serial.print(" | Error IMU2: ");
    Serial.print(error2);

    Serial.print(" | Error IMU3: ");
    Serial.println(error3);
  }

  Serial.print("Tiempo mala postura: ");
  Serial.print(tiempoMalaPostura);
  Serial.println(" s");

  Serial.print("Alertas: ");
  Serial.println(numeroAlertas);

  Serial.println("--------------------------------");

  //Calibración
  if (Serial.available())
  {
    char comando = Serial.read();

    if (comando == 'c' || comando == 'C')
    {
      calibrarPostura();
    }
  }

  delay(200);
}
