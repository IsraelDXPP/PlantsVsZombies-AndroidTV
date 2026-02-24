<div align="center">

# PvZ Touch

Mod de *Plants vs. Zombies* versión Android TV.

</div>

## Compilación

### Requisitos

* Android SDK Platform 34
* NDK r27c (27.2.12479018)
* CMake 3.20 o superior

### Pasos

1. Clona el repositorio:

```sh
git clone https://github.com/Dicot0721/PlantsVsZombies-AndroidTV.git
```

2. Copia los archivos de recursos en:

```
PlantsVsZombies-AndroidTV/app/src/main/assets/
```

3. Compila el proyecto:

* Desde Android Studio: usa el botón **Build**.
* Desde la terminal:

```sh
cd PlantsVsZombies-AndroidTV
./gradlew assembleDebug
```

### Compilación en modo Release

Crea un archivo `keystore.properties` en la raíz del proyecto con el siguiente formato:

```properties
storePassword=tuStorePassword
keyPassword=tuKeyPassword
keyAlias=tuKeyAlias
storeFile=rutaDelKeystore
```

---

## Contribución

### Estilo de código (C++)

* Funciones, tipos y conceptos: `PascalCase`
* Variables: `camelCase`
* Namespaces: `snake_case`
* Macros y constantes: `UPPER_CASE`

Se recomienda formatear el código antes de cada commit.

### Pull Requests

Enviar los PR a la rama `dev`.

---

## Licencia

Este proyecto está bajo la licencia GPL-3.0.

No está afiliado ni respaldado por Transmension, PopCap o Electronic Arts.
