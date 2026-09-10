# FOTA Injection Plugin para DLT Viewer

La estructura sigue el mismo modelo que los plugins visibles en la captura: una DLL independiente dentro de la carpeta `plugins` de DLT Viewer. El archivo generado se llama `fotainjectionplugin.dll` y el nombre visible en la pestaña **Plugin** será `FOTA Injection Plugin`.

Este proyecto es independiente de `flashear_fota_guiado.bat` y no modifica `dlt-viewer.exe`.

El plugin implementa la interfaz de control de DLT Viewer y llama a:

```cpp
QDltControl::sendInjection(connectionIndex, applicationId, contextId, serviceId, data)
```

## Comando

```text
send <connection-index> <application-id> <context-id> <service-id> <data>
```

Ejemplo:

```text
send 0 FOTA MAIN 5505 tcucpkg;package.iso;hash;/ota/package.iso
```

Para `5204`, `5200`, `5205` y `5218`, cambia el `service-id` y el contenido de `data` según corresponda.

## Compilacion

El equipo actual tiene CMake y MSVC 2022, pero no tiene instalado el paquete de desarrollo Qt5. DLT Viewer incluye DLLs de ejecucion, pero no los headers ni `Qt5Config.cmake` necesarios para compilar. En un entorno con Qt5 de desarrollo, CMake y el SDK de DLT Viewer:

```powershell
cmake -S . -B build -DDLT_VIEWER_SDK="C:\ruta\al\DLTViewer\sdk"
cmake --build build --config Release
```

El proyecto busca `qdlt.lib` dentro de `DLT_VIEWER_SDK/lib` y necesita las librerias Qt5 compatibles con la version de DLT Viewer.

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

## Limitacion actual

El plugin ya tiene el envio de bajo nivel implementado, pero todavia no modifica el BAT para invocarlo. Antes de automatizar el flujo completo hay que confirmar el indice de conexion que usa DLT Viewer y probar una injection controlada.
