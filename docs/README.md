<div align="center">

# PvZ Touch (Fork)

Mod de *Plants vs. Zombies* versión Android TV.

Este fork añade nuevos **cheats al mod menu** y elimina algunos elementos innecesarios de la versión original para hacerlo más limpio y ligero.

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

* Desde Android Studio: usa **Build**.
* Desde la terminal:

```sh
cd PlantsVsZombies-AndroidTV
./gradlew assembleDebug
```

### Compilación en modo Release

Crea un archivo `keystore.properties` en la raíz del proyecto:

```properties
storePassword=tuStorePassword
keyPassword=tuKeyPassword
keyAlias=tuKeyAlias
storeFile=rutaDelKeystore
```

---

## Contribuciones

* Mantener el formato consistente antes de hacer commit.
* Enviar Pull Requests a la rama `dev`.

---

## Licencia

Proyecto bajo licencia GPL-3.0.

No está afiliado ni respaldado por Transmension, PopCap o Electronic Arts.
