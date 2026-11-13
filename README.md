# Robot Siguelíneas AMR (ATtiny816): manual de instalación y uso

Guía paso a paso para programar el robot con el ATtiny816 usando **megaTinyCore (Spence Konde)**, basada en `PAR Arduino_v3_26-27.pdf` y probada en un equipo con Windows 11.

---

## 1. Requisitos

- **Arduino IDE**: el PDF indica la 2.3.10, pero también funciona con la **1.8.19** (probado). Descarga: https://www.arduino.cc/en/software/
- **Driver CH340**: el programador aparece como `USB-SERIAL CH340 (COMx)` en el Administrador de dispositivos. Si no aparece, instala el driver CH340.
- Robot con **batería cargada**.

## 2. Instalar megaTinyCore (Spence Konde)

1. `Archivo > Preferencias` y, en **"Gestor de URLs adicionales de tarjetas"**, pon:
   ```
   http://descartes.net/package_drazzy.com_index.json,http://drazzy.com/package_drazzy.com_index.json
   ```
2. `Herramientas > Placa > Gestor de tarjetas...`
3. Busca **megaTinyCore**, elige la de **Spence Konde** e instala la versión más reciente (probado con la 2.6.11).

### Errores típicos de instalación

| Problema | Solución |
|---|---|
| Alguien metió **ATTinyCore** (`Attycore`) u otro paquete de placas dentro de `Documentos\Arduino\libraries` | Sácalo de ahí. ATTinyCore **no sirve** para el ATtiny816 (es para ATtiny antiguos) y en `libraries` solo van librerías. |
| Sketches (`.ino`) guardados dentro de `Documentos\Arduino\libraries\...` | Muévelos a `Documentos\Arduino\` (fuera de `libraries`). Si no, sale el aviso `Librería inválida encontrada...`. |
| `Adafruit_NeoPixel.h: No such file or directory` (ejemplo LEDs) | `Programa > Incluir librería > Administrar bibliotecas`, busca **Adafruit NeoPixel** e instálala. |

## 3. Configuración en Herramientas

Todo por defecto **excepto**:

| Opción | Valor |
|---|---|
| Placa | `ATtiny3226/3216/1626/1616/1606/826/816/806/426/416/406` |
| Chip | `ATtiny816` |
| Clock | **`16 MHz internal`** (¡no 20 MHz!) |
| UPDI/Reset Pin Function | `UPDI (no reset pin)` |
| Programador | `SerialUPDI - 230400 baud` |
| Puerto | el `USB-SERIAL CH340` (en nuestro equipo, COM7) |

> **Clock:** con `20 MHz` el ejemplo LEDs da `#error "CPU SPEED NOT SUPPORTED"`. Usa siempre **16 MHz**.
>
> **UPDI/Reset:** la captura del PDF muestra "UPDI w/reset on PB4 (2-series only)", pero el ATtiny816 es **serie 1**, así que esa opción no le sirve. Además solo se aplica con "Quemar bootloader", que no hace falta.

## 4. Subir un programa

1. Conecta el programador CH340 al robot y al USB.
2. **Levanta el robot** (ruedas al aire): puede tener grabado un programa de motores anterior y empezar a moverse al encenderlo.
3. **Enciende el robot** con el interruptor general.
4. Abre el `.ino` y pulsa **Verificar** (✓).
5. Sube con **`Programa > Subir usando programador`** (`Ctrl+Mayús+U`). **No uses el botón normal de Subir.**
6. Si todo va bien, termina con:
   ```
   Verify successful. Data in flash matches data in specified hex-file
   ```

### Si falla la subida

| Error | Causa probable |
|---|---|
| `UPDI initialisation failed` | El robot está apagado, el conector del programador está mal puesto, la batería está baja (los motores girando hacen caer la tensión), el conmutador BT/SERIAL no está en la posición de programar o el Monitor Serie está abierto. |
| `CPU SPEED NOT SUPPORTED` | El Clock está en 20 MHz. Ponlo en 16 MHz. |
| No aparece el puerto | Falta el driver CH340, o desconecta y vuelve a conectar el USB. |

## 5. Programas de prueba (`Programas de prueba_26_27.rar`)

Descomprímelos en `Documentos\Arduino\`. Todos compilan para el ATtiny816 a 16 MHz:

| Programa | Qué prueba | Flash usada |
|---|---|---|
| `Motores` | Motor derecho: adelante 1 s, atrás 1 s (**levanta el robot**) | 9% |
| `LEDs` | LED RGB (pin 8): rojo, azul y amarillo, 1 s cada uno. Necesita Adafruit NeoPixel | 29% |
| `Buzzer` | Zumbador | 13% |
| `Musiquita_guay` | Melodía con el zumbador | 42% |
| `Ejemplo_ultrasonido` | Distancia del HC-SR04 por el Monitor Serie (9600 baudios) | 41% |
| `sensor_superficie` | Sensores ópticos GP2S700 (siguelíneas) | 43% |
| `servo_prueba` | Servo | 19% |

### Qué comprobar con `LEDs.ino`

1. La subida termina con `Verify successful`.
2. Las ruedas se paran (se ha borrado el programa anterior).
3. El LED va rojo, azul, amarillo, cambiando cada segundo.
   - Si los colores salen cambiados, cambia `NEO_GRB` por `NEO_RGB`.
   - Si van más rápidos o más lentos de 1 s, revisa el Clock (16 MHz).
   - Si no hay luz o el color no cambia, prueba con la librería `tinyNeoPixel`, que viene con megaTinyCore.

## 6. Pines principales (ATtiny816, numeración Arduino)

| Pin | Función | Pin | Función |
|---|---|---|---|
| 0 | Batería | 9 | BPWM (motor derecho) |
| 1-4 | Sensores CNY4..CNY1 | 10 | AIN (dirección) |
| 5 | Buzzer | 11 | BIN (dirección) |
| 6 / 7 | RXD / TXD | 12 | ECHO ultrasonido |
| 8 | LED RGB | 13 | TRIG ultrasonido |
| 14 | I/O | 15 | STBY driver motores |
| 16 | APWM (motor izquierdo) | 17 | UPDI (programación) |

(Tabla "ARDUINO" del esquemático `MAIN_ROBOT_V2.4` del PDF.)
