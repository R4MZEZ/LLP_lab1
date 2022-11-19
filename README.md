# Low-level programming lab#1

### *nix
способ сборки: `make clean build`

способ запуска: `./main <flag]> <filename> [parse-file]`

### win*
способ сборки: `make -f Makefile-win clean build`

способ запуска: `main.exe <flag> <filename> [parse-file]`

## Доступные флаги:

-n : создать новый файл с именем <filename> и запустить инициализацию паттерна.
  
  Пример: `make.exe -n file.txt`
  
-o : открыть существующий файл с именем <filename> и войти в интерактивный режим.
  
  Пример: `make.exe -o file.txt`
  
-p : создать новый файл с именем <filename>, запросить инициализацию паттерна и наполнить данными из файла <parse-file>
  
  Пример: `make.txt -p file.txt data-generator/out.txt`

  ## Генератор данных
  
  Находится в `data-generator/main.py`
  
  способ запуска: `py main.py <count> <[field1-name]=[field1-type] [field2-name]=[field2-type]...>`
  
  Пример: 
  ```
  py main.py 500 code=Integer name=String
  ```
