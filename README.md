# FOTA Injection Plugin para DLT Viewer

La estructura sigue el mismo modelo que los plugins visibles en la captura: una DLL independiente dentro de la carpeta `plugins` de DLT Viewer. El archivo generado se llama `fotainjectionplugin.dll` y el nombre visible en la pestaña **Plugin** será `FOTA Injection Plugin`.

Este proyecto es independiente de `flashear_fota_guiado.bat` y no modifica `dlt-viewer.exe`.

El plugin implementa la interfaz de control de DLT Viewer y llama a:

```cpp
QDltControl::sendInjection(connectionIndex, applicationId, contextId, serviceId, data)
```

## Comandos

```text
send <connection-index> <application-id> <context-id> <service-id> <data>
connect-send <connection-index> <application-id> <context-id> <service-id> <data>
connect-wait <connection-index> <expected-states> <timeout-seconds>
connect-send-wait <connection-index> <application-id> <context-id> <service-id> <expected-states> <timeout-seconds> <data>
```

Ejemplo:

```text
connect-send-wait 1 FOTA MAIN 5505 DISTRIBUTE_COMPLETE 600 tcucpkg;package.iso;hash;/ota/package.iso
```

Para `5204`, `5200`, `5205` y `5218`, cambia el `service-id` y el contenido de `data` según corresponda.

## Compilacion

El repositorio incluye una copia minima del SDK `qdlt` compatible con el DLT Viewer instalado: sus headers y `qdlt.lib`. Solo hace falta que el entorno de compilacion proporcione Qt5 de desarrollo.

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\ruta\a\Qt\5.15.2\msvc2019_64"
cmake --build build --config Release
```

El proyecto usa por defecto `third_party/dlt-sdk`. También puedes proporcionar otro SDK con `-DDLT_VIEWER_SDK=...`.

## Instalacion

1. Cierra DLT Viewer.
2. Copia `fotainjectionplugin.dll` a:

	```text
	C:\Program Files (x86)\DLTViewer\plugins
	```

3. Abre DLT Viewer.
4. En la pestaña `Plugin`, pulsa `Refresh` si está disponible.
5. Debe aparecer `FOTA Injection Plugin`, inicialmente como `Disabled`.
6. Selecciónalo y habilítalo igual que los plugins de la captura.

El filtro `FLASH_FOTA_2.dlf` es independiente y puede seguir cargándose a la vez.

## Espera de estados

Los comandos `connect-send-wait` y `connect-wait` esperan los estados FOTA indicados, en orden y recibidos como nuevas trazas con `Application ID = FOTA`, antes de finalizar. Los estados se separan con comas, por ejemplo `INSTALL_PROGRESS,INSTALL_COMPLETE`; el timeout admite de 1 a 7200 segundos.

`flashear_fota_guiado_plugin.bat` usa estos comandos para encadenar distribución, instalación, comprobación del vehículo, activación, espera de `StFota` y retorno a `IDLE`.
