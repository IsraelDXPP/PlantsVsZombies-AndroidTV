<div align="center">

# PvZ Touch (Fork)

Mod de *Plants vs. Zombies* versión Android TV.

Este fork mantenido en
[https://github.com/IsraelDXPP/PlantsVsZombies-AndroidTV](https://github.com/IsraelDXPP/PlantsVsZombies-AndroidTV)

añade nuevos **cheats al mod menu** y elimina elementos innecesarios de la versión original para hacerlo más limpio y optimizado.

</div>

---

## Compilación

### Requisitos

* Android SDK Platform 34
* NDK r27c (27.2.12479018)
* CMake 3.20 o superior

### Pasos

1. Clona este fork:

```sh
git clone https://github.com/IsraelDXPP/PlantsVsZombies-AndroidTV.git
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

Crea un archivo `keystore.properties` en la raíz del proyecto con el siguiente formato:

```properties
storePassword=tuStorePassword
keyPassword=tuKeyPassword
keyAlias=tuKeyAlias
storeFile=rutaDelKeystore
```

---

## Modificaciones y contribuciones

Se permiten modificaciones y forks del proyecto.

Sin embargo, **no se aceptan contribuciones directas, Pull Requests ni cambios externos** en este repositorio.

---

## Licencia

Proyecto bajo licencia GPL-3.0.

No está afiliado ni respaldado por Transmension, PopCap o Electronic Arts.
