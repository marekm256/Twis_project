# Twis – dvojkolesový mini Segway

Twin
Wheel
Inteligent
Segway

Cieľ: zhotoviť funkčný dvojkolesový samovyvažovací robot (mini Segway) riadený cez STM32 a bezdrôtové ovládanie.

---

## 1. Štruktúra projektu

Projekt je STM32CubeIDE projekt. Vlastné moduly sú v `Src/`:

- `Src/main.c` – hlavný program, integrácia všetkých modulov
- `Src/motor_control/`
  - `motor_control.c`
  - `motor_control.h`
- `Src/comm/`
  - `comm.c`
  - `comm.h`
- `Src/imu_mpu6050/`
  - `imu_mpu6050.c`
  - `imu_mpu6050.h`
- `Src/ultrasonic/`
  - `ultrasonic.c`
  - `ultrasonic.h`

---

## 2. Rozdelenie úloh

- **Marek**
  - rozbehať PWM + enkodéry v motor_control.c
  - jednoduché nastavovanie rýchlosti kolies
- **Miro**
  - zvoliť rozhranie na bezdrôtovú komunikáciu Twis ↔ PC
  - navrhnúť jednoduchý textový protokol pre ovládanie z PC
- **Gabika**
  - implementovať gyroskop + akcelerometer (MPU-6050)
  - načítanie dát z MPU-6050 cez I2C
  - pripraviť výpočet uhla náklonu
- **Ľubo**
  - ultrazvukový senzor (meranie vzdialenosti) na detekovanie prekážok
  - merať vzdialenosť (čas odozvy ECHO)
  - pripraviť hranicu „prekážka vpredu“

---

## 3. Ako začať (pre každého člena)

1. Naklonuj repozitár:
   ```bash
   git clone https://github.com/marekm256/Twis_project.git
   cd Twis_project

   git checkout -b feature/motor_control
   git checkout -b feature/comm       - vytvorenie vlastnej vetvy
   git checkout -b feature/imu
   git checkout -b feature/ultrasonic

2. Po vykonaní zmien:
    git add .
    git commit -m "Popis zmeny"

    prvý krát:
    git push --set-upstream origin feature/motor_control

    každý další krát:
    git push origin feature/motor_control

3. Následna sa priamo na stránke dá spojiť vytvorená vetva s hlavným kodom po dohode s Team leadrom.


